#include "base/package_api.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cinttypes>
#include <cstring>
#include <cstdlib>  // for exit
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <event2/event.h>

#include "backend.h"
#include "include/socket_err.h"
#include "packages/external/external.h"
#include "packages/sockets/socket_efuns.h"

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
extern int socketpair_win32(SOCKET socks[2], int make_overlapped);  // in socketpair.cc
#endif

#ifndef _WIN32
#include <sstream>
#include <spawn.h>
#include <sys/wait.h>
#endif

namespace {

/* Promise-form jobs (issue #1319): collect stdout/stderr, then settle when
 * BOTH the child socket has closed and waitpid/GetExitCode has reported
 * the return code. Keyed by LPC socket index. */
struct ExternalPromiseJob {
  promise_t* prom = nullptr;
  std::string output;
  LPC_INT exit_code = 0;
  bool io_done = false;
  bool status_done = false;
  bool aborted = false;
};

struct ChildWatch {
#ifdef _WIN32
  PROCESS_INFORMATION pi{};
  SOCKET child_sock = static_cast<SOCKET>(INVALID_SOCKET);
#else
  pid_t pid = -1;
#endif
};

std::unordered_map<int, ExternalPromiseJob*> g_external_jobs;
std::unordered_map<int, ChildWatch> g_child_watches;

std::mutex g_exit_mu;
struct ChildExitNote {
  int fd;
  LPC_INT code;
};
std::vector<ChildExitNote> g_exit_notes;

void reject_with_number(promise_t* p, LPC_INT n) {
  push_number(n);
  promise_settle(p, sp, 1);
  pop_stack();
}

void settle_and_drop_job(int fd) {
  auto it = g_external_jobs.find(fd);
  if (it == g_external_jobs.end()) {
    return;
  }
  ExternalPromiseJob* job = it->second;
  g_external_jobs.erase(it);

  if (job->aborted) {
    push_constant_string("*external process aborted");
    promise_settle(job->prom, sp, 1);
    pop_stack();
  } else {
    array_t* arr = allocate_array(2);
    arr->item[0].type = T_STRING;
    arr->item[0].subtype = STRING_MALLOC;
    arr->item[0].u.string = string_copy(job->output.c_str(), "external_promise");
    arr->item[1].u.number = job->exit_code;
    push_refed_array(arr);
    promise_settle(job->prom, sp, 0);
    pop_stack();
  }
  free_promise(job->prom);
  delete job;
}

void try_finish_job(int fd) {
  auto it = g_external_jobs.find(fd);
  if (it == g_external_jobs.end()) {
    return;
  }
  ExternalPromiseJob* job = it->second;
  if (job->aborted || (job->io_done && job->status_done)) {
    settle_and_drop_job(fd);
  }
}

void attach_external_job(int fd, promise_t* p) {
  auto* job = new ExternalPromiseJob{};
  job->prom = p;
  g_external_jobs[fd] = job;
}

void drain_child_exits() {
  std::vector<ChildExitNote> notes;
  {
    std::lock_guard<std::mutex> const lock(g_exit_mu);
    notes.swap(g_exit_notes);
  }
  for (auto& note : notes) {
    auto it = g_external_jobs.find(note.fd);
    if (it == g_external_jobs.end()) {
      continue;
    }
    it->second->exit_code = note.code;
    it->second->status_done = true;
    try_finish_job(note.fd);
  }
}

/* Called from the waitpid / WaitForSingleObject thread. Must not touch
 * LPC; the wall-time event runs drain_child_exits() on the main loop. */
void note_child_exit(int fd, LPC_INT code) {
  {
    std::lock_guard<std::mutex> const lock(g_exit_mu);
    g_exit_notes.push_back(ChildExitNote{fd, code});
  }
  add_walltime_event(std::chrono::milliseconds(0), TickEvent::callback_type([] { drain_child_exits(); }));
}

#ifndef _WIN32
void stash_posix_child(int fd, pid_t pid) { g_child_watches[fd] = ChildWatch{pid}; }
#else
void stash_win32_child(int fd, PROCESS_INFORMATION pi, SOCKET child_sock) {
  ChildWatch w;
  w.pi = pi;
  w.child_sock = child_sock;
  g_child_watches[fd] = w;
}
#endif

void watch_child(int fd) {
  auto it = g_child_watches.find(fd);
  if (it == g_child_watches.end()) {
    return;
  }
  ChildWatch w = it->second;
  g_child_watches.erase(it);

#ifndef _WIN32
  pid_t const pid = w.pid;
  std::thread([=]() {
    int status = 0;
    LPC_INT code = -1;
    do {
      const int s = waitpid(pid, &status, WUNTRACED | WCONTINUED);
      if (s == -1) {
        debug(external_start, "external_start: waitpid() error: %s (%d).\n", strerror(errno),
              errno);
        note_child_exit(fd, -1);
        return;
      }
      if (WIFEXITED(status)) {
        code = WEXITSTATUS(status);
        debug(external_start, "external_start: child %jd exited, status=%d\n", (intmax_t)pid,
              WEXITSTATUS(status));
      } else if (WIFSIGNALED(status)) {
        code = 128 + WTERMSIG(status);
        debug(external_start, "external_start: child %jd killed by signal %d\n", (intmax_t)pid,
              WTERMSIG(status));
      } else if (WIFSTOPPED(status)) {
        debug(external_start, "external_start: child %jd stopped by signal %d\n", (intmax_t)pid,
              WSTOPSIG(status));
      } else if (WIFCONTINUED(status)) {
        debug(external_start, "external_start: child %jd continued\n", (intmax_t)pid);
      }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    note_child_exit(fd, code);
  }).detach();
#else
  PROCESS_INFORMATION const processInfo = w.pi;
  SOCKET const child_sock = w.child_sock;
  std::thread([=]() {
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = static_cast<DWORD>(-1);
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    debug(external_start, "external_start: pid: %d exited with %d.\n", processInfo.dwProcessId,
          exitCode);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    evutil_closesocket(child_sock);
    note_child_exit(fd, static_cast<LPC_INT>(exitCode));
  }).detach();
#endif
}

std::string quote_argument(const std::string& arg) {
  if (arg.empty()) {
    return "\"\"";
  }
  if (arg.find_first_of(" \t\n\v\"") == std::string::npos) {
    return arg;
  }
  std::string res = "\"";
  // from
  // https://learn.microsoft.com/en-us/archive/blogs/twistylittlepassagesallalike/everyone-quotes-command-line-arguments-the-wrong-way
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
    // The socket slot above is fully provisioned (fd, callbacks, event
    // listeners, STATE_DATA_XFER). Tear it down so we don't leak the event
    // listeners / callback strings and leave a dangling half-open efun socket
    // pointing at sv[0]. socket_close() closes lpc_socks[fd].fd (== sv[0]);
    // clear sv[0] afterwards so the DEFER doesn't double-close it.
    socket_close(fd, SC_FORCE | SC_FINAL_CLOSE);
    sv[0] = -1;
    return EESOCKET;
  }

  evutil_closesocket(sv[1]);
  sv[1] = -1;
  sv[0] = -1;

  debug(external_start, "external_start: Launching external command '%s %s', pid: %jd.\n",
        external_cmd[which], args->type == T_STRING ? args->u.string : "<ARRAY>", (intmax_t)pid);

  /* Reaper starts after the caller attaches a promise job (if any), so a
   * fast child cannot post its exit code before the job exists. */
  stash_posix_child(fd, pid);
  return fd;
}
#endif

#ifdef _WIN32
int external_start(int which, svalue_t* args, svalue_t* arg1, svalue_t* arg2, svalue_t* arg3) {
  int fd;

  std::string cmd = external_cmd[which];
  // guard against long path with spaces.
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

  // Start the child process.
  if (!CreateProcessA(NULL,            // No module name (use command line)
                      cmdline.data(),  // Command line
                      NULL,            // Process handle not inheritable
                      NULL,            // Thread handle not inheritable
                      TRUE,            // Set handle inheritance to TRUE
                      0,               // No creation flags
                      NULL,            // Use parent's environment block
                      NULL,            // Use parent's starting directory
                      &si,             // Pointer to STARTUPINFO structure
                      &processInfo)    // Pointer to PROCESS_INFORMATION structure
  ) {
    error("CreateProcess() in external_start() failed: %s\n", strerror(errno));
    return EESOCKET;
  }
  debug(external_start, "external_start: Launching external command '%s', pid: %d.\n",
        cmdline.c_str(), processInfo.dwProcessId);

  stash_win32_child(fd, processInfo, sv[0]);
  return fd;
}
#endif

int external_promise_take_read(int fd, const char* data, int len) {
  auto it = g_external_jobs.find(fd);
  if (it == g_external_jobs.end()) {
    return 0;
  }
  if (data && len > 0) {
    auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);
    auto& out = it->second->output;
    size_t room = (max_string_length > 0 && static_cast<size_t>(max_string_length) > out.size())
                      ? static_cast<size_t>(max_string_length) - out.size()
                      : 0;
    if (room > 0) {
      out.append(data, std::min(static_cast<size_t>(len), room));
    }
  }
  return 1;
}

void external_promise_closed(int fd, int aborted) {
  auto it = g_external_jobs.find(fd);
  if (it == g_external_jobs.end()) {
    return;
  }
  if (aborted) {
    it->second->aborted = true;
  } else {
    it->second->io_done = true;
  }
  try_finish_job(fd);
}

void external_cleanup() {
  while (!g_external_jobs.empty()) {
    auto fd = g_external_jobs.begin()->first;
    g_external_jobs.begin()->second->aborted = true;
    settle_and_drop_job(fd);
  }
  g_child_watches.clear();
  std::lock_guard<std::mutex> const lock(g_exit_mu);
  g_exit_notes.clear();
}

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_external() {
  for (auto& entry : g_external_jobs) {
    if (entry.second->prom) {
      entry.second->prom->extra_ref++;
    }
  }
}
#endif

#ifdef F_EXTERNAL_START
void f_external_start() {
  /* Latch arity first: check_valid_socket() runs a master apply that
   * overwrites st_num_arg (same trap as f_async_read / f_async_db_exec). */
  int const num_arg = st_num_arg;
  svalue_t* arg = sp - num_arg + 1;
  int const promise_form = (num_arg == 2);

  if (num_arg != 2 && num_arg < 4) {
    error(
        "external_start: promise form takes two arguments; classic form needs "
        "read and write callbacks.\n");
  }

  if (!check_valid_socket("external", -1, current_object, "N/A", -1)) {
    st_num_arg = num_arg;
    if (promise_form) {
      promise_t* p = promise_alloc();
      reject_with_number(p, EESECURITY);
      pop_n_elems(num_arg);
      push_refed_promise(p);
      return;
    }
    pop_n_elems(num_arg - 1);
    sp->u.number = EESECURITY;
    return;
  }
  st_num_arg = num_arg;

  auto which = arg[0].u.number;
  if (--which < 0 || which > (g_num_external_cmds - 1) || !external_cmd[which]) {
    error("Bad argument 1 to external_start()\n");
  }

  int fd;
  if (promise_form) {
    /* Spawn first: Windows CreateProcess error()s, so no promise is live
     * across that unwind. Attach the job, then start the reaper so a
     * fast child cannot post its exit code before the job exists. */
    fd = external_start(which, arg + 1, nullptr, nullptr, nullptr);
    if (fd < 0) {
      promise_t* p = promise_alloc();
      reject_with_number(p, fd);
      pop_n_elems(num_arg);
      push_refed_promise(p);
      return;
    }
    promise_t* p = promise_alloc();
    p->ref++; /* the job's ref; push_refed_promise takes the other */
    attach_external_job(fd, p);
    watch_child(fd);
    pop_n_elems(num_arg);
    push_refed_promise(p);
    return;
  }

  fd = external_start(which, arg + 1, arg + 2, arg + 3, (num_arg == 5 ? arg + 4 : nullptr));
  if (fd >= 0) {
    watch_child(fd);
  }
  pop_n_elems(num_arg - 1);
  sp->u.number = fd;
}
#endif
