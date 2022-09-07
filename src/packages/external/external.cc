#include "base/package_api.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>  // for exit
#include <ctype.h>   // for isspace
#include <unistd.h>  // for fork
#include <thread>
#include <string>
#include <fmt/format.h>

#include <event2/event.h>

#include "include/socket_err.h"
#include "packages/sockets/socket_efuns.h"

#ifndef _WIN32
int external_start(int which, svalue_t *args, svalue_t *arg1, svalue_t *arg2, svalue_t *arg3) {
  evutil_socket_t sv[2];
  char *cmd;
  int fd;
  char **argv;
  pid_t ret;

  if (--which < 0 || which > (g_num_external_cmds - 1) || !external_cmd[which]) {
    error("Bad argument 1 to external_start()\n");
  }
  cmd = external_cmd[which];
  fd = find_new_socket();
  if (fd < 0) {
    return fd;
  }

  if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1) {
    return EESOCKET;
  }
  if (evutil_make_socket_nonblocking(sv[0]) == -1 || evutil_make_socket_nonblocking(sv[1]) == -1) {
    return EESOCKET;
  }
  ret = fork();
  if (ret == -1) {
    error("fork() in external_start() failed: %s\n", strerror(errno));
  }
  if (ret) {
    debug_message("Launching external command '%s %s', pid: %d.\n", external_cmd[which],
                  args->type == T_STRING ? args->u.string : "<ARRAY>", ret);

    auto sock = lpc_socks_get(fd);

    new_lpc_socket_event_listener(fd, sock, sv[0]);

    close(sv[1]);
    sock->fd = sv[0];
    sock->flags = S_EXTERNAL;
    set_read_callback(fd, arg1);
    set_write_callback(fd, arg2);
    set_close_callback(fd, arg3);
    sock->owner_ob = current_object;
    sock->mode = STREAM;
    sock->state = STATE_DATA_XFER;
    memset(reinterpret_cast<char *>(&sock->l_addr), 0, sizeof(sock->l_addr));
    memset(reinterpret_cast<char *>(&sock->r_addr), 0, sizeof(sock->r_addr));
    sock->owner_ob = current_object;
    sock->release_ob = NULL;
    sock->r_buf = NULL;
    sock->r_off = 0;
    sock->r_len = 0;
    sock->w_buf = NULL;
    sock->w_off = 0;
    sock->w_len = 0;

    current_object->flags |= O_EFUN_SOCKET;

    event_add(sock->ev_read, NULL);

    return fd;
  } else {
    int flag = 1;
    int i = 1;
    int n = 1;
    const char *p;
    char *arg;

    if (args->type == T_ARRAY) {
      n = args->u.arr->size;
    } else {
      p = args->u.string;

      while (*p) {
        if (isspace(*p)) {
          flag = 1;
        } else {
          if (flag) {
            n++;
            flag = 0;
          }
        }
        p++;
      }
    }

    argv =
        reinterpret_cast<char **>(DCALLOC(n + 1, sizeof(char *), TAG_TEMPORARY, "external args"));

    argv[0] = cmd;

    /* need writable version */
    if (args->type == T_ARRAY) {
      int j;
      svalue_t *sv = args->u.arr->item;

      for (j = 0; j < n; j++) {
        argv[i++] = alloc_cstring(sv[j].u.string, "external args");
      }
    } else {
      flag = 1;
      arg = alloc_cstring(args->u.string, "external args");
      while (*arg) {
        if (isspace(*arg)) {
          *arg = 0;
          flag = 1;
        } else {
          if (flag) {
            argv[i++] = arg;
            flag = 0;
          }
        }
        arg++;
      }
    }
    argv[i] = 0;

    close(sv[0]);
    for (i = 0; i < 5; i++) {
      if (external_port[i].port) {
        close(external_port[i].fd);  // close external ports
      }
    }
    dup2(sv[1], 0);
    dup2(sv[1], 1);
    dup2(sv[1], 2);
    execv(cmd, argv);
    exit(0);
    return 0;
  }
}
#endif

#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
extern int socketpair_win32(SOCKET socks[2], int make_overlapped);  // in socketpair.cc

int external_start(int which, svalue_t *args, svalue_t *arg1, svalue_t *arg2, svalue_t *arg3) {
  int fd;
  pid_t ret;

  if (--which < 0 || which > (g_num_external_cmds - 1) || !external_cmd[which]) {
    error("Bad argument 1 to external_start()\n");
  }

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
      argv.emplace_back(item.u.string);
    }
    cmdline += fmt::format("{}", fmt::join(argv.begin(), argv.end(), " "));
  } else {
    cmdline += std::string(args->u.string);
  }

  fd = find_new_socket();
  if (fd < 0) {
    return fd;
  }

  auto sock = lpc_socks_get(fd);

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
  memset(reinterpret_cast<char *>(&sock->l_addr), 0, sizeof(sock->l_addr));
  memset(reinterpret_cast<char *>(&sock->r_addr), 0, sizeof(sock->r_addr));
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
  debug_message("Launching external command '%s', pid: %d.\n", cmdline.c_str(),
                processInfo.dwProcessId);

  std::thread([=]() {
    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = -1;
    // Get the exit code.
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    debug_message("pid: %d exited with %d.\n", processInfo.dwProcessId, exitCode);
    CloseHandle(processInfo.hProcess);
    CloseHandle(processInfo.hThread);
    closesocket(sv[0]);
  }).detach();

  return fd;
}
#endif

#ifdef F_EXTERNAL_START
void f_external_start(void) {
  int fd, num_arg = st_num_arg;
  svalue_t *arg = sp - num_arg + 1;

  if (check_valid_socket("external", -1, current_object, "N/A", -1)) {
    fd = external_start(arg[0].u.number, arg + 1, arg + 2, arg + 3, (num_arg == 5 ? arg + 4 : 0));
    pop_n_elems(num_arg - 1);
    sp->u.number = fd;
  } else {
    pop_n_elems(num_arg - 1);
    sp->u.number = EESECURITY;
  }
}
#endif
