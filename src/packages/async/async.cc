#include "base/package_api.h"

#include "packages/async/async.h"

#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#if HAVE_DIRENT_H
#include <dirent.h>
#else
#define dirent direct
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#include <sys/param.h>  // for MAXPATHLEN
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <zlib.h>
#include <vm/internal/base/interpret.h>

#ifdef F_ASYNC_DB_EXEC
#include "packages/db/db.h"
#endif

#include "packages/core/file.h"  // check_valid_path, FIXME

namespace {

enum atypes { AREAD, AWRITE, AGETDIR, ADBEXEC, ADONE };

enum astates { BUSY, DONE };

struct Request {
  std::string path;
  int flags;
  int ret;
  int handle;
  std::string data;
  function_to_call_t* fun; /* callback form; null in the promise form */
  /* Promise form (issue #1319): settled at delivery instead of calling a
     callback -- fulfilled with the value the callback would have received,
     rejected with the failure value. Ref held by the request. */
  struct promise_t* prom = nullptr;
  struct Request* next;
  enum atypes type;
  int status;
  /* User context captured at registration so this_player() survives into
     the callback, like call_out() (issue #1104). Gated on the
     'this_player in call_out' setting. */
  object_t* command_giver = nullptr;
  /* Bound args trailing the callback (async_db_exec's spec allows a
     trailing `...`), heap-copied off the VM stack -- mirrors call_out()'s
     cop->vs (call_out.cc), since this callback fires long after the
     registering stack frame is gone. fun->args/fun->narg are repointed at
     this array's storage; null when there are no bound args. */
  array_t* bound_args = nullptr;
};

/* Capture the current user context on a new request (issue #1104). */
void capture_command_giver(struct Request* req) {
  if (CONFIG_INT(__RC_THIS_PLAYER_IN_CALL_OUT__) && command_giver) {
    req->command_giver = command_giver;
    add_ref(command_giver, "async: capture_command_giver");
  }
}

struct Work {
  struct Request* data;
  void* (*func)(struct Request*);
};

std::deque<struct Work*> reqs;
std::mutex reqs_lock;

// Works a worker thread is CURRENTLY processing: popped from reqs but not yet
// moved to finished_reqs. Guarded by reqs_lock so async_mark_request() (main
// thread, via check_memory) can account for their callback funptr /
// command_giver during that window -- otherwise a DEBUGMALLOC sweep that lands
// mid-processing false-flags the ref (Windows Debug hit this on async_read.lpc).
//
// This is a SET, not a single pointer: do_stuff() spawns a fresh worker thread
// whenever reqs is momentarily empty, which happens while an earlier worker is
// still inside a slow w->func() (it has already popped its request). So two or
// more workers can run at once; a single `current_work` pointer only tracked
// the last one and left the others' refs unaccounted -> flaky "Bad ref count"
// (gcc Debug hit this via the async_* tests firing in quick succession).
std::set<struct Work*> current_works;

std::deque<struct Request*> finished_reqs;
std::mutex finished_reqs_lock;

// FluffOS deadlocks because check_reqs() held finished_reqs_lock while
// executing an LPC async callback; if that callback queued another async
// operation, the main thread could wait for reqs_lock while a worker held
// reqs_lock and waited for finished_reqs_lock, freezing the event loop.
// This fix removes each completed request under the lock, records it as an
// active callback, releases the lock before invoking LPC, then removes it
// from the active set after cleanup; async_mark_request() also marks active
// requests so Debug reference checking remains correct. A regression
// exercises nested async callbacks and calls check_memory() from inside one,
// deterministically reproducing the old deadlock.

// Requests whose callbacks are executing on the main thread. check_reqs()
// removes each request from finished_reqs before invoking LPC, so keep the
// request visible to async_mark_request() until callback cleanup is complete.
// Guarded by finished_reqs_lock.
std::set<struct Request*> active_callbacks;

void thread_func() {
  Tracer::setThreadName("Package Async thread");

  ScopedTracer const tracer("Async thread loop");

  while (true) {
    struct Work* w = nullptr;
    {
      std::lock_guard<std::mutex> const lock(reqs_lock);
      if (reqs.empty()) {
        return;
      }
      w = reqs.front();
      reqs.pop_front();
      current_works.insert(w);  // in-flight: keep it accountable while we process
    }

    if (w) {
      {
        ScopedTracer const work_tracer("Async thread work", EventCategory::DEFAULT,
                                       [=] { return json{{"type", w->data->type}}; });

        w->func(w->data);
      }
      if (w->data->status == DONE) {
        // Clear current_work and publish to finished_reqs atomically
        // w.r.t. async_mark_request (which takes both locks in this
        // order) so the funptr is marked exactly once across the move.
        std::lock_guard<std::mutex> const rlock(reqs_lock);
        std::lock_guard<std::mutex> const flock(finished_reqs_lock);
        current_works.erase(w);
        finished_reqs.push_back(w->data);
        delete w;
      } else {
        std::lock_guard<std::mutex> const lock(reqs_lock);
        current_works.erase(w);
        reqs.push_back(w);
      }

      add_walltime_event(std::chrono::milliseconds(0),
                         TickEvent::callback_type([] { check_reqs(); }));
    }
  }
}

void do_stuff(void* (*func)(struct Request*), struct Request* data) {
  std::lock_guard<std::mutex> const lock(reqs_lock);

  if (reqs.empty()) {
    std::thread(thread_func).detach();
  }

  auto* i = new Work;
  i->func = func;
  i->data = data;

  reqs.push_back(i);
}

void* gzreadthread(struct Request* req) {
  gzFile file = gzopen(req->path.c_str(), "rb");
  req->ret = gzread(file, (void*)(req->data.data()), req->data.size());
  req->status = DONE;
  gzclose(file);
  return nullptr;
}

int aio_gzread(struct Request* req) {
  req->status = BUSY;
  do_stuff(gzreadthread, req);
  return 0;
}

void* gzwritethread(struct Request* req) {
  int const fd = open(req->path.c_str(),
                      req->flags & 1 ? O_CREAT | O_WRONLY | O_TRUNC : O_CREAT | O_WRONLY | O_APPEND,
                      S_IRWXU | S_IRWXG);
  gzFile file = gzdopen(fd, "wb");
  req->ret = gzwrite(file, (void*)(req->data.data()), req->data.size());
  req->status = DONE;
  gzclose(file);
  return nullptr;
}

int aio_gzwrite(struct Request* req) {
  req->status = BUSY;
  do_stuff(gzwritethread, req);
  return 0;
}

void* writethread(struct Request* req) {
  int const fd = open(req->path.c_str(),
                      req->flags & 1 ? O_CREAT | O_WRONLY | O_TRUNC : O_CREAT | O_WRONLY | O_APPEND,
                      S_IRWXU | S_IRWXG);

  req->ret = write(fd, req->data.data(), req->data.size());

  req->status = DONE;
  close(fd);
  return nullptr;
}

int aio_write(struct Request* req) {
  req->status = BUSY;
  do_stuff(writethread, req);
  return 0;
}

void* readthread(struct Request* req) {
  int const fd = open(req->path.c_str(), O_RDONLY);
  if (fd < 0) {
    req->ret = -1;
    req->status = DONE;
    return nullptr;
  }
  // Bound the read by the buffer's ACTUAL size, not max_size() (~2^62) --
  // the caller sizes req->data before dispatching (mirror gzreadthread just
  // above, which correctly uses .size()). A short read shrinks the buffer;
  // a negative return leaves it untouched.
  auto size = read(fd, (void*)(req->data.data()), req->data.size());
  close(fd);
  if (size >= 0) {
    req->data.resize(size);
  }
  req->ret = size;
  req->status = DONE;
  return nullptr;
}

int aio_read(struct Request* req) {
  req->status = BUSY;
  do_stuff(readthread, req);
  return 0;
}

}  // namespace

#ifdef F_ASYNC_DB_EXEC
pthread_mutex_t* db_mut = nullptr;

void* dbexecthread(struct Request* req) {
  ScopedTracer const work_tracer("db_exec", EventCategory::DEFAULT,
                                 [=] { return json{req->data}; });

  pthread_mutex_lock(db_mut);
  // see add_db_exec
  db_t* db = find_db_conn(req->handle);
  int ret = -1;
  if (db && db->type->execute) {
    if (db->type->cleanup) {
      db->type->cleanup(&(db->c));
    }

    ret = db->type->execute(&(db->c), req->data.c_str());
    if (ret == -1) {
      if (db->type->error) {
        char* tmp = db->type->error(&(db->c));
        req->path = std::string(tmp);
        FREE_MSTR(tmp);
      } else {
        req->path = "Unknown error";
      }
    }
  } else {
    req->path = std::string("No database exec function!");
  }
  pthread_mutex_unlock(db_mut);

  req->ret = ret;
  req->status = DONE;
  return nullptr;
}

int aio_db_exec(struct Request* req) {
  req->status = BUSY;
  do_stuff(dbexecthread, req);
  return 0;
}
#endif

#ifdef F_ASYNC_GETDIR
void* getdirthread(struct Request* req) {
  ScopedTracer const work_tracer("getdir", EventCategory::DEFAULT, [=] { return json{req->path}; });

  DIR* dirp = nullptr;
  if ((dirp = opendir(req->path.c_str())) == nullptr) {
    req->ret = 0;
    req->status = DONE;
    return nullptr;
  }
  /*
   * Count files
   */
  int i = 0;
  for (auto* de = readdir(dirp); de; de = readdir(dirp)) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    // The buffer is used as an array of `struct dirent`, indexed by i, so it
    // must grow by sizeof(dirent) per entry -- growing by sizeof(dirent*) (a
    // pointer, ~8 bytes) undersized it and memcpy of the ~280-byte dirent
    // overflowed the heap once a directory held enough entries.
    req->data.resize(static_cast<size_t>(i + 1) * sizeof(struct dirent));
    memcpy(&((dirent*)(req->data.data()))[i], de, sizeof(*de));
    i++;
  }

  closedir(dirp);

  req->ret = i;
  req->status = DONE;
  return nullptr;
}

int aio_getdir(struct Request* req) {
  req->status = BUSY;
  do_stuff(getdirthread, req);
  return 0;
}

#endif

/* Denied valid_read/valid_write path, shared by the add_* helpers: no
   request will ever be created to free what the efun already handed us
   (the ref-bumped callback, or the promise), so release it before
   unwinding. */
static void denied(function_to_call_t* fun, promise_t* prom) {
  if (fun) {
    free_funp(fun->f.fp);
    delete fun;
  }
  if (prom) {
    free_promise(prom);
  }
  error("permission denied\n");
}

int add_read(const char* fname, function_to_call_t* fun, promise_t* prom) {
  const auto read_file_max_size = CONFIG_INT(__MAX_READ_FILE_SIZE__);

  if (fname) {
    auto* req = new Request();
    // printf("fname: %s\n", fname);
    req->data.resize(read_file_max_size);
    req->fun = fun;
    req->prom = prom;
    req->type = AREAD;
    capture_command_giver(req);
    req->path = std::string(fname);
    return aio_gzread(req);
  }
  denied(fun, prom);

  return 1;
}

#ifdef F_ASYNC_GETDIR
int add_getdir(const char* fname, function_to_call_t* fun, promise_t* prom) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  if (fname) {
    // printf("fname: %s\n", fname);
    auto* req = new Request();
    req->data.resize(max_array_size);
    req->fun = fun;
    req->prom = prom;
    req->type = AGETDIR;
    capture_command_giver(req);
    req->path = fname;
    return aio_getdir(req);
  }
  denied(fun, prom);

  return 1;
}
#endif

int add_write(const char* fname, const char* buf, int size, char flags, function_to_call_t* fun,
              promise_t* prom) {
  if (!fname) {
    denied(fun, prom);
  }

  auto* req = new Request();
  req->data = std::string(buf, size);
  req->fun = fun;
  req->prom = prom;
  req->type = AWRITE;
  capture_command_giver(req);
  req->flags = flags;
  req->path = std::string(fname);
  if (flags & 2) {
    return aio_gzwrite(req);
  }
  return aio_write(req);
}

#ifdef F_ASYNC_DB_EXEC
int add_db_exec(int handle, const char* sql, function_to_call_t* fun, array_t* bound_args,
                promise_t* prom) {
  auto* req = new Request();
  req->fun = fun;
  req->prom = prom;
  req->bound_args = bound_args;
  req->type = ADBEXEC;
  capture_command_giver(req);
  req->handle = handle;
  req->data = sql;
  return aio_db_exec(req);
}
#endif

/* Deliver a finished request's result, the one value pushed on `result`:
   the callback form calls req->fun with it, the promise form settles
   req->prom with it. The value stays on the stack while the promise
   settles, so an error() in a cancel handler cannot leak it. */
static void deliver_result(LPCCall& result, struct Request* req, int rejected) {
  if (req->prom) {
    promise_settle(req->prom, result.arg(0).sv(), rejected);
    result.discard();
    return;
  }
  set_eval(max_eval_cost);
  result.safe_call(req->fun);
}

void handle_read(struct Request* req) {
  int const val = req->ret;
  LPCCall result;
  if (val < 0) {
    result.push(val);
    deliver_result(result, req, /*rejected=*/1);
    return;
  }
  char* file = new_string(val, "read_file_async: str");
  memcpy(file, (char*)(req->data.data()), val);
  file[val] = 0;
  result.push(Value::str_malloced(file));
  deliver_result(result, req, 0);
}

#ifdef F_ASYNC_GETDIR
void handle_getdir(struct Request* req) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int ret_size = req->ret;
  if (ret_size > max_array_size) {
    ret_size = max_array_size;
  }
  array_t* ret = allocate_empty_array(ret_size);
  if (ret_size > 0) {
    for (int i = 0; i < ret_size; i++) {
      auto de = ((struct dirent*)req->data.data())[i];
      svalue_t* vp = &(ret->item[i]);
      vp->type = T_STRING;
      vp->subtype = STRING_MALLOC;
      vp->u.string = string_copy(de.d_name, "encode_stat");
    }

    qsort((void*)ret->item, ret_size, sizeof ret->item[0],
          [](const void* p1, const void* p2) -> int {
            auto* x = (svalue_t*)p1;
            auto* y = (svalue_t*)p2;

            return strcmp(x->u.string, y->u.string);
          });
  }

  LPCCall result;
  result.push(Value::refed(ret));
  deliver_result(result, req, 0);
}
#endif

void handle_write(struct Request* req) {
  int const val = req->ret;
  LPCCall result;
  if (val < 0) {
    result.push(val);
    deliver_result(result, req, /*rejected=*/1);
    return;
  }
  result.push(Value::undefined());
  deliver_result(result, req, 0);
}

void handle_db_exec(struct Request* req) {
  int const val = req->ret;
  LPCCall result;
  if (val == -1) {
    result.push(Value::str_copy(req->path.c_str()));
  } else {
    result.push(val);
  }
  /* Promise form: -1 is a rejection (the error string), anything else is
     the row count the callback would have been called with. The callback
     form is unchanged -- it still receives either value as its argument. */
  deliver_result(result, req, req->prom && val == -1);
}

void check_reqs() {
  ScopedTracer const tracer("Async callback");

  while (true) {
    struct Request* req;
    {
      std::lock_guard<std::mutex> const lock(finished_reqs_lock);
      if (finished_reqs.empty()) {
        return;
      }

      req = finished_reqs.front();
      finished_reqs.pop_front();
      active_callbacks.insert(req);
    }

    enum atypes const type = (req->type);
    req->type = ADONE;
    /* Restore the user context captured at registration (issue #1104). */
    object_t* new_command_giver = nullptr;
    if (req->command_giver && !(req->command_giver->flags & O_DESTRUCTED)) {
      new_command_giver = req->command_giver;
    }
    save_command_giver(new_command_giver);
    switch (type) {
      case AREAD:
        handle_read(req);
        break;
      case AWRITE:
        handle_write(req);
        break;
#ifdef F_ASYNC_GETDIR
      case AGETDIR:
        handle_getdir(req);
        break;
#endif
#ifdef F_ASYNC_DB_EXEC
      case ADBEXEC:
        handle_db_exec(req);
        break;
#endif
      case ADONE:
        // must have had an error while handling it before.
        break;
      default:
        fatal("unknown async type\n");
    }
    restore_command_giver();
    if (req->command_giver) {
      free_object(&req->command_giver, "async: check_reqs");
    }
    if (req->fun) { /* null in the promise form */
      free_funp(req->fun->f.fp);
      delete req->fun;
    }
    if (req->prom) {
      free_promise(req->prom);
    }
    if (req->bound_args) {
      free_array(req->bound_args);
    }

    {
      std::lock_guard<std::mutex> const lock(finished_reqs_lock);
      active_callbacks.erase(req);
    }
    delete req;
  }
}

void complete_all_asyncio() {
  while (true) {
    std::lock_guard<std::mutex> const lock(reqs_lock);

    if (reqs.empty()) {
      break;
    }
  }
  check_reqs();
}

#ifdef F_ASYNC_READ

Value f_async_read(EfunArgs& f, StringArg file, std::optional<funptr_t*> callback) {
  function_to_call_t* fun = nullptr;
  promise_t* prom = nullptr;

  /* Before any ownership is taken: check_valid_path() pushes args and runs
   * a master apply, either of which can error() -- with a released callback
   * ref or a fresh promise in a raw local, that unwind would leak them
   * (AGENTS.md section 4). Nothing below this line applies, so the returned
   * string stays valid until add_read() copies it. The path is the FIRST
   * argument -- the optional callback is the last when one was passed.
   * The form is decided by the callback parameter (not a global), so the
   * master apply cannot clobber form-detection or process_callback. */
  const char* path = check_valid_path(file.c_str(), current_object, "read_file", 0);
  if (callback) {
    /* The callback parameter is typed funptr_t*: the efun thunk has already
     * rejected a string (or anything else) there, so process_callback()
     * always fills cb->f.fp, never the string-callback form. */
    std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
    f.process_callback(1, cb.get());
    cb->f.fp->hdr.ref++;
    f.pop_back(1);
    fun = cb.release();
  } else {
    /* promise form: no callback, deliver by settling this (issue #1319) */
    prom = promise_alloc();
  }

  add_read(path, fun, prom);
  if (prom) {
    prom->ref++; /* the request's ref stays; this one is the return value */
    return Value::refed(prom);
  }
  return 0;
}
#endif

#ifdef F_ASYNC_WRITE
Value f_async_write(EfunArgs& f, StringArg file, StringArg buf, LPC_INT flags,
                    std::optional<funptr_t*> callback) {
  function_to_call_t* fun = nullptr;
  promise_t* prom = nullptr;

  /* before any ownership is taken -- see f_async_read(); the path is the
   * FIRST of 3 or 4 arguments */
  const char* path = check_valid_path(file.c_str(), current_object, "write_file", 1);
  if (callback) {
    /* typed funptr_t*: see f_async_read() */
    std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
    f.process_callback(3, cb.get());
    cb->f.fp->hdr.ref++;
    f.pop_back(1);
    fun = cb.release();
  } else {
    prom = promise_alloc();
  }

  add_write(path, buf.c_str(), buf.size(), flags, fun, prom);
  if (prom) {
    prom->ref++;
    return Value::refed(prom);
  }
  return 0;
}
#endif

#ifdef F_ASYNC_GETDIR
Value f_async_getdir(EfunArgs& f, StringArg dir, std::optional<funptr_t*> callback) {
  function_to_call_t* fun = nullptr;
  promise_t* prom = nullptr;

  /* before any ownership is taken -- see f_async_read(); the path is the
   * FIRST argument */
  const char* path = check_valid_path(dir.c_str(), current_object, "get_dir", 0);
  if (callback) {
    /* typed funptr_t*: see f_async_read() */
    std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
    f.process_callback(1, cb.get());
    cb->f.fp->hdr.ref++;
    f.pop_back(1);
    fun = cb.release();
  } else {
    prom = promise_alloc();
  }

  add_getdir(path, fun, prom);
  if (prom) {
    prom->ref++;
    return Value::refed(prom);
  }
  return 0;
}
#endif
#ifdef F_ASYNC_DB_EXEC
// The funptr ref has to survive error() between the ref++ and hand-off.
// A DEFER lambda is the wrong tool here: Cppcheck reads it as if it ran at
// the macro, while the pointer is still null, and reports that the later
// assignment is unused (Codacy on #1444). A destructor runs at scope exit,
// which is the moment the flag and the pointer are actually set.
struct AsyncCallbackRef {
  funptr_t* fp = nullptr;
  bool handed_off = false;
  ~AsyncCallbackRef() {
    if (fp != nullptr && !handed_off) {
      free_funp(fp);
    }
  }
  AsyncCallbackRef() = default;
  AsyncCallbackRef(const AsyncCallbackRef&) = delete;
  AsyncCallbackRef& operator=(const AsyncCallbackRef&) = delete;
};

Value f_async_db_exec(EfunArgs& f, LPC_INT handle, StringArg sql, std::optional<Arg> callback) {
  // Two forms (issue #1319), same split as async_read():
  //   async_db_exec(handle, sql, callback, ...)  -> classic, returns 0
  //   async_db_exec(handle, sql)                 -> promise for the row count
  //
  // process_callback() locates the callback from the window's arity
  // (arg = window base + narg). Parse the callback before
  // valid_database() so ownership/ordering stays clear; the old
  // global-arity bug is gone, but the promise vs callback form split still
  // keys on the callback's presence before any apply.
  std::unique_ptr<function_to_call_t> cb;
  AsyncCallbackRef cb_ref;
  array_t* bound_args = nullptr;
  bool bound_args_handed_off = false;
  // Registered before any error() that can follow the array ref. The
  // pointer stays null until that ref exists, and [&] sees the later
  // assignment (AGENTS.md section 4).
  DEFER {
    if (bound_args && !bound_args_handed_off) free_array(bound_args);
  };

  if (callback) {
    cb.reset(new function_to_call_t);
    f.process_callback(2, cb.get());

    // The callback fires long after this stack frame is gone (a DB round
    // trip on a worker thread, then a later game tick) -- unlike the
    // synchronous consumers of function_to_call_t (pcre, array
    // map/filter/sort), a string-named callback (cb->ob set) has no
    // reference-counted lifetime of its own here. Reject that form cleanly
    // instead of type-confusing cb->f.str (a char*) as a funptr_t*.
    if (cb->ob != nullptr) {
      error("async_db_exec: callback must be a function pointer, not a string.\n");
    }

    cb->f.fp->hdr.ref++;
    cb_ref.fp = cb->f.fp;

    // Bound args trailing the callback (the spec's `...`) are only safe to
    // keep past this stack frame if their ownership is transferred off the
    // VM stack -- mirroring call_out()'s cop->vs (call_out.cc): a bitwise
    // copy, no ref bump (the copy IS the new owning reference).
    if (cb->narg > 0) {
      bound_args = allocate_empty_array(cb->narg);
      memcpy(bound_args->item, cb->args, sizeof(svalue_t) * cb->narg);
      cb->args = bound_args->item;
    }

    // The bound args' stack slots were bitwise-transferred into bound_args
    // above (not ref-bumped), so forget them before the pop so it does not
    // free them -- same idiom as int_call_out().
    for (int i = 3; i < f.argc(); i++) {
      f.forget(i);
    }
    // remove the (forgotten) bound args and the callback; the window is
    // handle, sql (`callback` pointed into a released slot from here on)
    f.pop_back(f.argc() - 2);
  }

  array_t* info;
  info = allocate_empty_array(1);
  info->item[0].type = T_STRING;
  info->item[0].subtype = STRING_MALLOC;
  info->item[0].u.string = string_copy(sql.c_str(), "f_db_exec");
  valid_database("exec", info);

  db_t* db;
  db = find_db_conn(handle);
  if (!db) {
    error("Attempt to exec on an invalid database handle\n");
  }

  if (!db_mut) {
    db_mut = (pthread_mutex_t*)DMALLOC(sizeof(pthread_mutex_t), TAG_PERMANENT, "async_db_exec");
    pthread_mutex_init(db_mut, nullptr);
  }

  // Allocated only after the calls that error(), so a bad handle or a
  // denied valid_database() cannot leak it. The request holds this ref;
  // the returned svalue gets its own below.
  promise_t* prom = nullptr;
  if (!cb) {
    prom = promise_alloc();
  }

  cb_ref.handed_off = true;
  bound_args_handed_off = true;
  add_db_exec(handle, sql.c_str(), cb ? cb.release() : nullptr, bound_args, prom);
  if (prom) {
    prom->ref++;
    return Value::refed(prom);
  }
  return 0;
}
#endif

void async_mark_request() {
#ifdef DEBUGMALLOC_EXTENSIONS
  std::lock_guard<std::mutex> const lock(reqs_lock);
  std::lock_guard<std::mutex> const flock(finished_reqs_lock);

  for (auto& work : reqs) {
    auto* req = work->data;
    if (req->fun != nullptr) {
      req->fun->f.fp->hdr.extra_ref++;
    }
    if (req->prom != nullptr) {
      req->prom->extra_ref++;
    }
    if (req->command_giver != nullptr) {
      req->command_giver->extra_ref++;
    }
    if (req->bound_args != nullptr) {
      req->bound_args->extra_ref++;
    }
  }

  // Requests a worker is mid-processing (popped from reqs, not yet in
  // finished_reqs); guarded by reqs_lock, held above. There may be several
  // concurrent workers, so mark every in-flight work, not just one.
  for (auto* work : current_works) {
    if (work->data->fun != nullptr) {
      work->data->fun->f.fp->hdr.extra_ref++;
    }
    if (work->data->prom != nullptr) {
      work->data->prom->extra_ref++;
    }
    if (work->data->command_giver != nullptr) {
      work->data->command_giver->extra_ref++;
    }
    if (work->data->bound_args != nullptr) {
      work->data->bound_args->extra_ref++;
    }
  }

  for (auto& req : finished_reqs) {
    if (req->fun != nullptr) {
      req->fun->f.fp->hdr.extra_ref++;
    }
    if (req->prom != nullptr) {
      req->prom->extra_ref++;
    }
    if (req->command_giver != nullptr) {
      req->command_giver->extra_ref++;
    }
    if (req->bound_args != nullptr) {
      req->bound_args->extra_ref++;
    }
  }

  for (auto* req : active_callbacks) {
    if (req->fun != nullptr) {
      req->fun->f.fp->hdr.extra_ref++;
    }
    if (req->command_giver != nullptr) {
      req->command_giver->extra_ref++;
    }
    if (req->bound_args != nullptr) {
      req->bound_args->extra_ref++;
    }
  }
#endif
}
