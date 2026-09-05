#include "base/package_api.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <cstdlib>  // for exit
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <event2/event.h>

#include "backend.h"
#include "base/internal/strutils.h"
#include "include/socket_err.h"
#include "packages/external/external.h"
#include "packages/sockets/socket_efuns.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <fcntl.h>
extern int socketpair_win32(SOCKET socks[2], int make_overlapped);  // in socketpair.cc
#endif

#ifndef _WIN32
#include <csignal>
#include <fcntl.h>
#include <sstream>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace {

constexpr int kMaxHandles = 1024;
constexpr int kPipeBuf = 4096;

enum class HandleState : uint8_t { Created, Running, Done };

struct PipeWatch {
  int handle;
  int stream; /* 0 = stdout, 1 = stderr */
};

struct ExternalHandle {
  object_t* owner = nullptr;
  int cmd_index = -1; /* 0-based */
  std::vector<std::string> args;
  HandleState state = HandleState::Created;
  promise_t* prom = nullptr;
  /* Omit-callback form: not visible to LPC. Fulfill with
   * ({ stdout+stderr, exit_code }) and free the slot. */
  bool ephemeral = false;
  std::string out;
  std::string err;
  LPC_INT exit_code = -1;
  bool out_eof = false;
  bool err_eof = false;
  bool status_done = false;
  evutil_socket_t out_fd = -1;
  evutil_socket_t err_fd = -1;
  struct event* ev_out = nullptr;
  struct event* ev_err = nullptr;
  PipeWatch* out_watch = nullptr;
  PipeWatch* err_watch = nullptr;
#ifndef _WIN32
  pid_t pid = -1;
#else
  PROCESS_INFORMATION pi{};
#endif
};

std::vector<ExternalHandle*> g_handles; /* handle id = index + 1 */

std::mutex g_exit_mu;
struct ChildExitNote {
  int handle;
  LPC_INT code;
};
std::vector<ChildExitNote> g_exit_notes;

void reject_with_number(promise_t* p, LPC_INT n) {
  push_number(n);
  promise_settle(p, sp, 1);
  pop_stack();
}

void append_capped(std::string& dest, const char* data, size_t len) {
  if (!data || len == 0) {
    return;
  }
  auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);
  size_t room = (max_string_length > 0 && static_cast<size_t>(max_string_length) > dest.size())
                    ? static_cast<size_t>(max_string_length) - dest.size()
                    : 0;
  if (room > 0) {
    dest.append(data, std::min(len, room));
  }
}

ExternalHandle* lookup_handle(int id, int require_owner) {
  if (id < 1 || id > static_cast<int>(g_handles.size()) || !g_handles[id - 1]) {
    error("Bad argument 1 to external efun: invalid handle.\n");
  }
  ExternalHandle* h = g_handles[id - 1];
  if (require_owner && h->owner != current_object) {
    error("Bad argument 1 to external efun: handle is not owned by this object.\n");
  }
  return h;
}

int alloc_handle_id() {
  for (size_t i = 0; i < g_handles.size(); i++) {
    if (!g_handles[i]) {
      return static_cast<int>(i + 1);
    }
  }
  if (static_cast<int>(g_handles.size()) >= kMaxHandles) {
    error("external_create: too many handles.\n");
  }
  g_handles.push_back(nullptr);
  return static_cast<int>(g_handles.size());
}

void parse_cmd_args(svalue_t* args, std::vector<std::string>* extra) {
  extra->clear();
  if (args->type == T_ARRAY) {
    for (int i = 0; i < args->u.arr->size; i++) {
      auto item = args->u.arr->item[i];
      if (item.type != T_STRING) {
        error("Bad argument list item %d to external efun\n", i);
      }
      extra->emplace_back(item.u.string);
    }
  } else {
#ifndef _WIN32
    std::istringstream iss(args->u.string);
    std::string item;
    while (std::getline(iss, item, ' ')) {
      if (!item.empty()) {
        extra->push_back(item);
      }
    }
#else
    extra->emplace_back(args->u.string);
#endif
  }
}

int validate_cmd_index(LPC_INT which) {
  int idx = static_cast<int>(which) - 1;
  if (idx < 0 || idx > (g_num_external_cmds - 1) || !external_cmd[idx]) {
    error("Bad argument 1 to external efun: unconfigured command.\n");
  }
  return idx;
}

void close_pipe_fd(evutil_socket_t* fd) {
  if (*fd < 0) {
    return;
  }
#ifdef _WIN32
  evutil_closesocket(*fd);
#else
  close(*fd);
#endif
  *fd = -1;
}

void free_pipe_events(ExternalHandle* h) {
  if (h->ev_out) {
    event_free(h->ev_out);
    h->ev_out = nullptr;
  }
  if (h->ev_err) {
    event_free(h->ev_err);
    h->ev_err = nullptr;
  }
  delete h->out_watch;
  h->out_watch = nullptr;
  delete h->err_watch;
  h->err_watch = nullptr;
  close_pipe_fd(&h->out_fd);
  close_pipe_fd(&h->err_fd);
}

void fulfill_handle_promise(int id) {
  ExternalHandle* h = g_handles[id - 1];
  if (!h->prom) {
    if (h->ephemeral) {
      delete h;
      g_handles[id - 1] = nullptr;
    }
    return;
  }
  promise_t* p = h->prom;
  h->prom = nullptr;
  if (h->ephemeral) {
    std::string output = h->out;
    if (!h->err.empty()) {
      output += h->err;
    }
    array_t* arr = allocate_array(2);
    arr->item[0].type = T_STRING;
    arr->item[0].subtype = STRING_MALLOC;
    arr->item[0].u.string = string_copy(output.c_str(), "external_promise");
    arr->item[1].u.number = h->exit_code;
    push_refed_array(arr);
    promise_settle(p, sp, 0);
    pop_stack();
    free_promise(p);
    delete h;
    g_handles[id - 1] = nullptr;
    return;
  }
  push_number(0);
  promise_settle(p, sp, 0);
  pop_stack();
  free_promise(p);
}

void try_finish_handle(int id) {
  if (id < 1 || id > static_cast<int>(g_handles.size()) || !g_handles[id - 1]) {
    return;
  }
  ExternalHandle* h = g_handles[id - 1];
  if (h->state != HandleState::Running) {
    return;
  }
  if (!h->out_eof || !h->err_eof || !h->status_done) {
    return;
  }
  h->state = HandleState::Done;
  free_pipe_events(h);
  fulfill_handle_promise(id);
}

void abort_handle(int id, int kill_child) {
  if (id < 1 || id > static_cast<int>(g_handles.size()) || !g_handles[id - 1]) {
    return;
  }
  ExternalHandle* h = g_handles[id - 1];
  if (h->state == HandleState::Running && kill_child) {
#ifndef _WIN32
    if (h->pid > 0) {
      kill(h->pid, SIGTERM);
    }
#else
    if (h->pi.hProcess) {
      TerminateProcess(h->pi.hProcess, 1);
    }
#endif
  }
  free_pipe_events(h);
  if (h->prom) {
    push_constant_string("*external process aborted");
    promise_settle(h->prom, sp, 1);
    pop_stack();
    free_promise(h->prom);
    h->prom = nullptr;
  }
  h->state = HandleState::Done;
}

void destroy_handle(int id, int kill_child) {
  if (id < 1 || id > static_cast<int>(g_handles.size()) || !g_handles[id - 1]) {
    return;
  }
  abort_handle(id, kill_child);
  delete g_handles[id - 1];
  g_handles[id - 1] = nullptr;
}

void drain_child_exits() {
  std::vector<ChildExitNote> notes;
  {
    std::lock_guard<std::mutex> const lock(g_exit_mu);
    notes.swap(g_exit_notes);
  }
  for (auto& note : notes) {
    if (note.handle < 1 || note.handle > static_cast<int>(g_handles.size()) ||
        !g_handles[note.handle - 1]) {
      continue;
    }
    ExternalHandle* h = g_handles[note.handle - 1];
    h->exit_code = note.code;
    h->status_done = true;
    try_finish_handle(note.handle);
  }
}

void note_child_exit(int handle, LPC_INT code) {
  {
    std::lock_guard<std::mutex> const lock(g_exit_mu);
    g_exit_notes.push_back(ChildExitNote{handle, code});
  }
  add_walltime_event(std::chrono::milliseconds(0),
                     TickEvent::callback_type([] { drain_child_exits(); }));
}

void on_handle_pipe_read(evutil_socket_t fd, short /*what*/, void* arg) {
  auto* watch = static_cast<PipeWatch*>(arg);
  int const id = watch->handle;
  if (id < 1 || id > static_cast<int>(g_handles.size()) || !g_handles[id - 1]) {
    return;
  }
  ExternalHandle* h = g_handles[id - 1];
  char buf[kPipeBuf];
#ifdef _WIN32
  int cc = recv(fd, buf, sizeof(buf) - 1, 0);
#else
  int cc = static_cast<int>(read(fd, buf, sizeof(buf) - 1));
#endif
  if (cc > 0) {
    buf[cc] = '\0';
    auto res = u8_sanitize(buf);
    append_capped(watch->stream == 0 ? h->out : h->err, res.c_str(), res.size());
    return;
  }
  if (cc < 0) {
#ifdef _WIN32
    int err = evutil_socket_geterror(fd);
    if (err == WSAEWOULDBLOCK) {
      return;
    }
#else
    if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
      return;
    }
#endif
  }
  if (watch->stream == 0) {
    h->out_eof = true;
  } else {
    h->err_eof = true;
  }
  try_finish_handle(id);
}

void arm_pipe_reader(ExternalHandle* h, int id, int stream, evutil_socket_t fd) {
  auto* watch = new PipeWatch{id, stream};
  struct event* ev =
      event_new(g_event_base, fd, EV_READ | EV_PERSIST, on_handle_pipe_read, watch);
  if (stream == 0) {
    h->out_watch = watch;
    h->ev_out = ev;
    h->out_fd = fd;
  } else {
    h->err_watch = watch;
    h->ev_err = ev;
    h->err_fd = fd;
  }
  event_add(ev, nullptr);
}

#ifndef _WIN32
int spawn_handle_posix(ExternalHandle* h, int id) {
  std::vector<std::string> argv_data = {std::string(external_cmd[h->cmd_index])};
  argv_data.insert(argv_data.end(), h->args.begin(), h->args.end());
  std::vector<char*> argv;
  for (auto& a : argv_data) {
    argv.push_back(a.data());
  }
  argv.push_back(nullptr);

  int outp[2] = {-1, -1};
  int errp[2] = {-1, -1};
  if (pipe(outp) != 0 || pipe(errp) != 0) {
    if (outp[0] >= 0) {
      close(outp[0]);
    }
    if (outp[1] >= 0) {
      close(outp[1]);
    }
    if (errp[0] >= 0) {
      close(errp[0]);
    }
    if (errp[1] >= 0) {
      close(errp[1]);
    }
    return EESOCKET;
  }
  if (evutil_make_socket_nonblocking(outp[0]) == -1 ||
      evutil_make_socket_nonblocking(errp[0]) == -1) {
    close(outp[0]);
    close(outp[1]);
    close(errp[0]);
    close(errp[1]);
    return EESOCKET;
  }

  posix_spawn_file_actions_t file_actions;
  int ret = posix_spawn_file_actions_init(&file_actions);
  if (ret != 0) {
    close(outp[0]);
    close(outp[1]);
    close(errp[0]);
    close(errp[1]);
    return EESOCKET;
  }
  DEFER { posix_spawn_file_actions_destroy(&file_actions); };

  ret = posix_spawn_file_actions_adddup2(&file_actions, outp[1], 1) ||
        posix_spawn_file_actions_adddup2(&file_actions, errp[1], 2) ||
        posix_spawn_file_actions_addopen(&file_actions, 0, "/dev/null", O_RDONLY, 0) ||
        posix_spawn_file_actions_addclose(&file_actions, outp[0]) ||
        posix_spawn_file_actions_addclose(&file_actions, errp[0]);
  if (ret != 0) {
    close(outp[0]);
    close(outp[1]);
    close(errp[0]);
    close(errp[1]);
    return EESOCKET;
  }

  pid_t pid;
  char* newenviron[] = {nullptr};
  ret = posix_spawn(&pid, argv[0], &file_actions, nullptr, argv.data(), newenviron);
  if (ret != 0) {
    debug(external_start, "external_start: posix_spawn() error: %s\n", strerror(ret));
    close(outp[0]);
    close(outp[1]);
    close(errp[0]);
    close(errp[1]);
    return EESOCKET;
  }
  close(outp[1]);
  close(errp[1]);
  h->pid = pid;

  arm_pipe_reader(h, id, 0, outp[0]);
  arm_pipe_reader(h, id, 1, errp[0]);

  std::thread([id, pid]() {
    int status = 0;
    LPC_INT code = -1;
    do {
      const int s = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (s == -1) {
        note_child_exit(id, -1);
        return;
      }
      if (WIFEXITED(status)) {
        code = WEXITSTATUS(status);
      } else if (WIFSIGNALED(status)) {
        code = 128 + WTERMSIG(status);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    note_child_exit(id, code);
  }).detach();
  return 0;
}
#endif

#ifdef _WIN32
std::string quote_argument(const std::string& arg) {
  if (arg.empty()) {
    return "\"\"";
  }
  if (arg.find_first_of(" \t\n\v\"") == std::string::npos) {
    return arg;
  }
  std::string res = "\"";
  for (auto It = arg.begin();; ++It) {
    unsigned NumberBackslashes = 0;
    while (It != arg.end() && *It == '\\') {
      ++It;
      ++NumberBackslashes;
    }
    if (It == arg.end()) {
      res.append(NumberBackslashes * 2, '\\');
      break;
    } else if (*It == '"') {
      res.append(NumberBackslashes * 2 + 1, '\\');
      res.push_back(*It);
    } else {
      res.append(NumberBackslashes, '\\');
      res.push_back(*It);
    }
  }
  res.push_back('"');
  return res;
}

int spawn_handle_win32(ExternalHandle* h, int id) {
  std::string cmd = external_cmd[h->cmd_index];
  cmd = trim(cmd, " ");
  if (cmd[0] != '"') {
    cmd = fmt::format("\"{}\"", cmd);
  }
  std::string cmdline = cmd + " ";
  std::vector<std::string> quoted;
  for (auto& a : h->args) {
    quoted.emplace_back(quote_argument(a));
  }
  cmdline += fmt::to_string(fmt::join(quoted.begin(), quoted.end(), " "));

  SOCKET out_sv[2];
  SOCKET err_sv[2];
  if (socketpair_win32(out_sv, 0) != 0 || socketpair_win32(err_sv, 0) != 0) {
    return EESOCKET;
  }
  if (evutil_make_socket_nonblocking(out_sv[1]) == -1 ||
      evutil_make_socket_nonblocking(err_sv[1]) == -1) {
    evutil_closesocket(out_sv[0]);
    evutil_closesocket(out_sv[1]);
    evutil_closesocket(err_sv[0]);
    evutil_closesocket(err_sv[1]);
    return EESOCKET;
  }

  SECURITY_ATTRIBUTES sa{sizeof(sa), nullptr, TRUE};
  HANDLE nul = CreateFileA("NUL", GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, &sa,
                           OPEN_EXISTING, 0, nullptr);

  STARTUPINFOA si = {sizeof(si)};
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;
  si.hStdInput = nul;
  si.hStdOutput = reinterpret_cast<HANDLE>(out_sv[0]);
  si.hStdError = reinterpret_cast<HANDLE>(err_sv[0]);
  PROCESS_INFORMATION processInfo{};

  if (!CreateProcessA(NULL, cmdline.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &processInfo)) {
    if (nul != INVALID_HANDLE_VALUE) {
      CloseHandle(nul);
    }
    evutil_closesocket(out_sv[0]);
    evutil_closesocket(out_sv[1]);
    evutil_closesocket(err_sv[0]);
    evutil_closesocket(err_sv[1]);
    return EESOCKET;
  }
  if (nul != INVALID_HANDLE_VALUE) {
    CloseHandle(nul);
  }
  evutil_closesocket(out_sv[0]);
  evutil_closesocket(err_sv[0]);
  h->pi = processInfo;

  arm_pipe_reader(h, id, 0, out_sv[1]);
  arm_pipe_reader(h, id, 1, err_sv[1]);

  std::thread([id, processInfo]() {
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = static_cast<DWORD>(-1);
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    note_child_exit(id, static_cast<LPC_INT>(exitCode));
  }).detach();
  return 0;
}
#endif

int spawn_handle(ExternalHandle* h, int id) {
#ifndef _WIN32
  return spawn_handle_posix(h, id);
#else
  return spawn_handle_win32(h, id);
#endif
}

/* Spawn a Created handle. On failure the handle is deleted and the
 * returned promise is already rejected (caller just pushes it). */
promise_t* start_created_handle(ExternalHandle* h, int id) {
  promise_t* p = promise_alloc();
  int const rc = spawn_handle(h, id);
  if (rc < 0) {
    g_handles[id - 1] = nullptr;
    delete h;
    reject_with_number(p, rc);
    return p;
  }
  h->state = HandleState::Running;
  h->prom = p;
  p->ref++;
  return p;
}

#ifndef _WIN32
template <typename Out>
void split(const std::string& s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    *result++ = item;
  }
}
#endif

}  // namespace

#ifndef _WIN32
int external_start(int which, svalue_t* args, svalue_t* arg1, svalue_t* arg2, svalue_t* arg3) {
  std::vector<std::string> newargs_data = {std::string(external_cmd[which])};
  if (args->type == T_ARRAY) {
    for (int i = 0; i < args->u.arr->size; i++) {
      auto item = args->u.arr->item[i];
      if (item.type != T_STRING) {
        error("Bad argument list item %d to external_start()\n", i);
      }
      newargs_data.push_back(item.u.string);
    }
  } else {
    split(std::string(args->u.string), ' ', std::back_inserter(newargs_data));
  }

  std::vector<char*> newargs;
  for (auto& arg : newargs_data) {
    newargs.push_back(arg.data());
  }
  newargs.push_back(nullptr);

  posix_spawn_file_actions_t file_actions;
  int ret = posix_spawn_file_actions_init(&file_actions);
  if (ret != 0) {
    debug(external_start, "external_start: posix_spawn_file_actions_init() error: %s\n",
          strerror(ret));
    return EESOCKET;
  }
  DEFER { posix_spawn_file_actions_destroy(&file_actions); };

  evutil_socket_t sv[2];
  if (evutil_socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1) {
    return EESOCKET;
  }
  DEFER {
    if (sv[0] > 0) {
      evutil_closesocket(sv[0]);
    }
    if (sv[1] > 0) {
      evutil_closesocket(sv[1]);
    }
  };
  if (evutil_make_socket_nonblocking(sv[0]) == -1 || evutil_make_socket_nonblocking(sv[1]) == -1) {
    return EESOCKET;
  }
  ret = posix_spawn_file_actions_adddup2(&file_actions, sv[1], 0) ||
        posix_spawn_file_actions_adddup2(&file_actions, sv[1], 1) ||
        posix_spawn_file_actions_adddup2(&file_actions, sv[1], 2);
  if (ret != 0) {
    debug(external_start, "external_start: posix_spawn_file_actions_adddup2() error: %s\n",
          strerror(ret));
    return EESOCKET;
  }

  int fd = find_new_socket();
  if (fd < 0) {
    return fd;
  }

  auto* sock = lpc_socks_get(fd);
  new_lpc_socket_event_listener(fd, sock, sv[0]);

  sock->fd = sv[0];
  sock->flags = S_EXTERNAL;
  set_read_callback(fd, arg1);
  set_write_callback(fd, arg2);
  set_close_callback(fd, arg3);
  sock->owner_ob = current_object;
  sock->mode = STREAM;
  sock->state = STATE_DATA_XFER;
  memset(reinterpret_cast<char*>(&sock->l_addr), 0, sizeof(sock->l_addr));
  memset(reinterpret_cast<char*>(&sock->r_addr), 0, sizeof(sock->r_addr));
  sock->owner_ob = current_object;
  sock->release_ob = nullptr;
  sock->r_buf = nullptr;
  sock->r_off = 0;
  sock->r_len = 0;
  sock->w_buf = nullptr;
  sock->w_off = 0;
  sock->w_len = 0;

  current_object->flags |= O_EFUN_SOCKET;

  event_add(sock->ev_write, nullptr);
  event_add(sock->ev_read, nullptr);

  pid_t pid;
  char* newenviron[] = {nullptr};
  ret = posix_spawn(&pid, newargs[0], &file_actions, nullptr, newargs.data(), newenviron);
  if (ret) {
    debug(external_start, "external_start: posix_spawn() error: %s\n", strerror(ret));
    socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
    sv[0] = -1;
    return EESOCKET;
  }

  evutil_closesocket(sv[1]);
  sv[1] = -1;
  sv[0] = -1;

  debug(external_start, "external_start: Launching external command '%s %s', pid: %jd.\n",
        external_cmd[which], args->type == T_STRING ? args->u.string : "<ARRAY>", (intmax_t)pid);

  std::thread([=]() {
    int status;
    do {
      const int s = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (s == -1) {
        return;
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }).detach();

  return fd;
}
#endif

#ifdef _WIN32
int external_start(int which, svalue_t* args, svalue_t* arg1, svalue_t* arg2, svalue_t* arg3) {
  int fd;

  std::string cmd = external_cmd[which];
  cmd = trim(cmd, " ");
  if (cmd[0] != '"') {
    cmd = fmt::format("\"{}\"", cmd);
  }
  std::string cmdline = cmd + " ";

  if (args->type == T_ARRAY) {
    std::vector<std::string> argv(args->u.arr->size);
    for (int i = 0; i < args->u.arr->size; i++) {
      auto item = args->u.arr->item[i];
      if (item.type != T_STRING) {
        error("Bad argument list item %d to external_start()\n", i);
      }
      argv.emplace_back(quote_argument(item.u.string));
    }
    cmdline += fmt::to_string(fmt::join(argv.begin(), argv.end(), " "));
  } else {
    cmdline += std::string(args->u.string);
  }

  fd = find_new_socket();
  if (fd < 0) {
    return fd;
  }

  auto* sock = lpc_socks_get(fd);

  SOCKET sv[2];
  socketpair_win32(sv, 0);

  new_lpc_socket_event_listener(fd, sock, sv[1]);

  sock->fd = sv[1];
  sock->flags = S_EXTERNAL;
  set_read_callback(fd, arg1);
  set_write_callback(fd, arg2);
  set_close_callback(fd, arg3);
  sock->owner_ob = current_object;
  sock->mode = STREAM;
  sock->state = STATE_DATA_XFER;
  memset(reinterpret_cast<char*>(&sock->l_addr), 0, sizeof(sock->l_addr));
  memset(reinterpret_cast<char*>(&sock->r_addr), 0, sizeof(sock->r_addr));
  sock->owner_ob = current_object;
  sock->release_ob = NULL;
  sock->r_buf = NULL;
  sock->r_off = 0;
  sock->r_len = 0;
  sock->w_buf = NULL;
  sock->w_off = 0;
  sock->w_len = 0;

  current_object->flags |= O_EFUN_SOCKET;

  event_add(sock->ev_write, NULL);
  event_add(sock->ev_read, NULL);

  STARTUPINFOA si = {sizeof(si)};
  si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
  si.wShowWindow = SW_HIDE;
  si.hStdInput = reinterpret_cast<HANDLE>(sv[0]);
  si.hStdError = reinterpret_cast<HANDLE>(sv[0]);
  si.hStdOutput = reinterpret_cast<HANDLE>(sv[0]);
  PROCESS_INFORMATION processInfo{};

  if (!CreateProcessA(NULL, cmdline.data(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &processInfo)) {
    error("CreateProcess() in external_start() failed: %s\n", strerror(errno));
    return EESOCKET;
  }
  debug(external_start, "external_start: Launching external command '%s', pid: %d.\n",
        cmdline.c_str(), processInfo.dwProcessId);

  std::thread([=]() {
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = -1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    evutil_closesocket(sv[0]);
  }).detach();

  return fd;
}
#endif

void external_owner_destructed(object_t* ob) {
  for (int i = 0; i < static_cast<int>(g_handles.size()); i++) {
    if (g_handles[i] && g_handles[i]->owner == ob) {
      destroy_handle(i + 1, /*kill_child=*/1);
    }
  }
}

void external_cleanup() {
  for (int i = 0; i < static_cast<int>(g_handles.size()); i++) {
    if (g_handles[i]) {
      destroy_handle(i + 1, /*kill_child=*/1);
    }
  }
  g_handles.clear();
  std::lock_guard<std::mutex> const lock(g_exit_mu);
  g_exit_notes.clear();
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_external() {
  for (auto* h : g_handles) {
    if (h && h->prom) {
      h->prom->extra_ref++;
    }
  }
}
#endif

#ifdef F_EXTERNAL_CREATE
void f_external_create() {
  int const num_arg = st_num_arg;
  svalue_t* arg = sp - num_arg + 1;

  if (!check_valid_socket("external", -1, current_object, "N/A", -1)) {
    st_num_arg = num_arg;
    error("external_create: permission denied.\n");
  }
  st_num_arg = num_arg;

  int const cmd = validate_cmd_index(arg[0].u.number);
  std::vector<std::string> extra;
  parse_cmd_args(arg + 1, &extra);

  int const id = alloc_handle_id();
  auto* h = new ExternalHandle{};
  h->owner = current_object;
  h->cmd_index = cmd;
  h->args = std::move(extra);
  g_handles[id - 1] = h;

  pop_n_elems(num_arg);
  push_number(id);
}
#endif

#ifdef F_EXTERNAL_START
void f_external_start() {
  /* Latch arity first: check_valid_socket() runs a master apply that
   * overwrites st_num_arg (same trap as f_async_read). */
  int const num_arg = st_num_arg;
  svalue_t* arg = sp - num_arg + 1;

  if (num_arg == 1) {
    ExternalHandle* h = lookup_handle(static_cast<int>(arg[0].u.number), /*require_owner=*/1);
    int const id = static_cast<int>(arg[0].u.number);
    if (h->state != HandleState::Created) {
      error("external_start: handle has already been started.\n");
    }
    if (!check_valid_socket("external", -1, current_object, "N/A", -1)) {
      st_num_arg = num_arg;
      promise_t* p = promise_alloc();
      reject_with_number(p, EESECURITY);
      pop_n_elems(num_arg);
      push_refed_promise(p);
      return;
    }
    st_num_arg = num_arg;

    int const rc = spawn_handle(h, id);
    promise_t* p = promise_alloc();
    if (rc < 0) {
      reject_with_number(p, rc);
      pop_n_elems(num_arg);
      push_refed_promise(p);
      return;
    }
    h->state = HandleState::Running;
    h->prom = p;
    p->ref++;
    pop_n_elems(num_arg);
    push_refed_promise(p);
    return;
  }

  if (num_arg == 2) {
    /* Issue #1319 omit-callback form: same efun, no callbacks, promise of
     * ({ output, exit_code }). Classic 4/5-arg path below is unchanged. */
    if (!check_valid_socket("external", -1, current_object, "N/A", -1)) {
      st_num_arg = num_arg;
      promise_t* p = promise_alloc();
      reject_with_number(p, EESECURITY);
      pop_n_elems(num_arg);
      push_refed_promise(p);
      return;
    }
    st_num_arg = num_arg;

    auto which = arg[0].u.number;
    if (--which < 0 || which > (g_num_external_cmds - 1) || !external_cmd[which]) {
      error("Bad argument 1 to external_start()\n");
    }

    std::vector<std::string> extra;
    parse_cmd_args(arg + 1, &extra);

    int const id = alloc_handle_id();
    auto* h = new ExternalHandle{};
    h->owner = current_object;
    h->cmd_index = static_cast<int>(which);
    h->args = std::move(extra);
    h->ephemeral = true;
    g_handles[id - 1] = h;

    promise_t* p = start_created_handle(h, id);
    pop_n_elems(num_arg);
    push_refed_promise(p);
    return;
  }

  if (num_arg != 4 && num_arg != 5) {
    error(
        "external_start: omit the callbacks for the promise form, pass a "
        "handle from external_create(), or pass the classic read/write "
        "callbacks.\n");
  }

  if (!check_valid_socket("external", -1, current_object, "N/A", -1)) {
    st_num_arg = num_arg;
    pop_n_elems(num_arg - 1);
    sp->u.number = EESECURITY;
    return;
  }
  st_num_arg = num_arg;

  auto which = arg[0].u.number;
  if (--which < 0 || which > (g_num_external_cmds - 1) || !external_cmd[which]) {
    error("Bad argument 1 to external_start()\n");
  }

  int fd = external_start(which, arg + 1, arg + 2, arg + 3, (num_arg == 5 ? arg + 4 : nullptr));
  pop_n_elems(num_arg - 1);
  sp->u.number = fd;
}
#endif

#ifdef F_EXTERNAL_STDOUT
void f_external_stdout() {
  ExternalHandle* h = lookup_handle(static_cast<int>(sp->u.number), /*require_owner=*/1);
  copy_and_push_string(h->out.c_str());
  assign_svalue(sp - 1, sp);
  pop_stack();
}
#endif

#ifdef F_EXTERNAL_STDERR
void f_external_stderr() {
  ExternalHandle* h = lookup_handle(static_cast<int>(sp->u.number), /*require_owner=*/1);
  copy_and_push_string(h->err.c_str());
  assign_svalue(sp - 1, sp);
  pop_stack();
}
#endif

#ifdef F_EXTERNAL_EXIT_CODE
void f_external_exit_code() {
  ExternalHandle* h = lookup_handle(static_cast<int>(sp->u.number), /*require_owner=*/1);
  sp->u.number = h->exit_code;
}
#endif

#ifdef F_EXTERNAL_CLOSE
void f_external_close() {
  int const id = static_cast<int>(sp->u.number);
  lookup_handle(id, /*require_owner=*/1);
  destroy_handle(id, /*kill_child=*/1);
  pop_stack();
}
#endif
