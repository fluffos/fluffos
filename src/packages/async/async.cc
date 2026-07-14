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
  function_to_call_t* fun;
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
  auto size = read(fd, (void*)(req->data.data()), req->data.max_size());
  close(fd);
  req->data.resize(size);
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

int add_read(const char* fname, function_to_call_t* fun) {
  const auto read_file_max_size = CONFIG_INT(__MAX_READ_FILE_SIZE__);

  if (fname) {
    auto* req = new Request();
    // printf("fname: %s\n", fname);
    req->data.resize(read_file_max_size);
    req->fun = fun;
    req->type = AREAD;
    capture_command_giver(req);
    req->path = std::string(fname);
    return aio_gzread(req);
  }
  // Denied path: no request will ever be created to free the callback the
  // efun already ref-bumped and handed us, so release it before unwinding.
  free_funp(fun->f.fp);
  delete fun;
  error("permission denied\n");

  return 1;
}

#ifdef F_ASYNC_GETDIR
int add_getdir(const char* fname, function_to_call_t* fun) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  if (fname) {
    // printf("fname: %s\n", fname);
    auto* req = new Request();
    req->data.resize(max_array_size);
    req->fun = fun;
    req->type = AGETDIR;
    capture_command_giver(req);
    req->path = fname;
    return aio_getdir(req);
  }
  // Denied path: free the callback the efun already ref-bumped and handed us.
  free_funp(fun->f.fp);
  delete fun;
  error("permission denied\n");

  return 1;
}
#endif

int add_write(const char* fname, const char* buf, int size, char flags, function_to_call_t* fun) {
  if (!fname) {
    // Denied path: free the callback the efun already ref-bumped and handed us.
    free_funp(fun->f.fp);
    delete fun;
    error("permission denied\n");
  }

  auto* req = new Request();
  req->data = std::string(buf, size);
  req->fun = fun;
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
int add_db_exec(int handle, const char* sql, function_to_call_t* fun, array_t* bound_args) {
  auto* req = new Request();
  req->fun = fun;
  req->bound_args = bound_args;
  req->type = ADBEXEC;
  capture_command_giver(req);
  req->handle = handle;
  req->data = sql;
  return aio_db_exec(req);
}
#endif

void handle_read(struct Request* req) {
  int const val = req->ret;
  if (val < 0) {
    push_number(val);
    set_eval(max_eval_cost);
    safe_call_efun_callback(req->fun, 1);
    return;
  }
  char* file = new_string(val, "read_file_async: str");
  memcpy(file, (char*)(req->data.data()), val);
  file[val] = 0;
  push_malloced_string(file);
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
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

  push_refed_array(ret);
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}
#endif

void handle_write(struct Request* req) {
  int const val = req->ret;
  if (val < 0) {
    push_number(val);
    set_eval(max_eval_cost);
    safe_call_efun_callback(req->fun, 1);
    return;
  }
  push_undefined();
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}

void handle_db_exec(struct Request* req) {
  int const val = req->ret;
  if (val == -1) {
    copy_and_push_string(req->path.c_str());
  } else {
    push_number(val);
  }
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}

void check_reqs() {
  ScopedTracer const tracer("Async callback");

  std::lock_guard<std::mutex> const lock(finished_reqs_lock);
  while (!finished_reqs.empty()) {
    auto* req = finished_reqs.front();
    finished_reqs.pop_front();

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
    free_funp(req->fun->f.fp);
    if (req->bound_args) {
      free_array(req->bound_args);
    }
    delete req->fun;
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

void f_async_read() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(1, cb.get(), F_ASYNC_READ);
  // A `mixed` argument bypasses the spec's compile-time `function` check;
  // process_efun_callback() then sets cb->ob (string-callback form) rather
  // than cb->f.fp, and the ref++ below would type-confuse cb->f.str (a
  // char*) as a funptr_t*.
  if (cb->ob != nullptr) {
    error("async_read: callback must be a function pointer, not a string.\n");
  }
  cb->f.fp->hdr.ref++;
  pop_stack();

  add_read(check_valid_path(sp->u.string, current_object, "read_file", 0), cb.release());
  pop_stack();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(3, cb.get(), F_ASYNC_WRITE);
  if (cb->ob != nullptr) {
    error("async_write: callback must be a function pointer, not a string.\n");
  }
  cb->f.fp->hdr.ref++;
  pop_stack();

  add_write(check_valid_path((sp - 2)->u.string, current_object, "write_file", 1),
            (sp - 1)->u.string, SVALUE_STRLEN((sp - 1)), sp->u.number, cb.release());
  pop_3_elems();
}
#endif

#ifdef F_ASYNC_GETDIR
void f_async_getdir() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(1, cb.get(), F_ASYNC_GETDIR);
  if (cb->ob != nullptr) {
    error("async_getdir: callback must be a function pointer, not a string.\n");
  }
  cb->f.fp->hdr.ref++;
  pop_stack();

  add_getdir(check_valid_path(sp->u.string, current_object, "get_dir", 0), cb.release());
  pop_stack();
}
#endif
#ifdef F_ASYNC_DB_EXEC
void f_async_db_exec() {
  // process_efun_callback() locates the callback via the GLOBAL
  // st_num_arg (arg = sp - st_num_arg + 1 + narg), so it MUST run before
  // valid_database() below -- that apply runs master LPC which leaves
  // st_num_arg clobbered. Reversing them made ftc->f.fp read the wrong
  // stack slot and crash at the ref++ once a real db handle existed
  // (found by async_db_exec.lpc on the SQLite CI build). unique_ptr owns
  // the struct and a scope guard releases the funptr ref, so an error()
  // unwind before hand-off leaks neither (AGENTS.md section 4).
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(2, cb.get(), F_ASYNC_DB_EXEC);

  // async_db_exec's callback fires long after this stack frame is gone (a
  // DB round trip on a worker thread, then a later game tick) -- unlike
  // the synchronous consumers of function_to_call_t (pcre, array
  // map/filter/sort), a string-named callback (cb->ob set) has no
  // reference-counted lifetime of its own here. Reject that form cleanly
  // instead of type-confusing cb->f.str (a char*) as a funptr_t*.
  if (cb->ob != nullptr) {
    error("async_db_exec: callback must be a function pointer, not a string.\n");
  }

  cb->f.fp->hdr.ref++;
  funptr_t* cb_fp = cb->f.fp;
  bool handed_off = false;
  DEFER {
    if (!handed_off) free_funp(cb_fp);
  };

  // Bound args trailing the callback (the spec's `...`) are only safe to
  // keep past this stack frame if their ownership is transferred off the
  // VM stack -- mirroring call_out()'s cop->vs (call_out.cc): a bitwise
  // copy, no ref bump (the copy IS the new owning reference).
  array_t* bound_args = nullptr;
  if (cb->narg > 0) {
    bound_args = allocate_empty_array(cb->narg);
    memcpy(bound_args->item, cb->args, sizeof(svalue_t) * cb->narg);
    cb->args = bound_args->item;
  }
  bool bound_args_handed_off = false;
  DEFER {
    if (bound_args && !bound_args_handed_off) free_array(bound_args);
  };

  // The bound args' stack slots were bitwise-transferred into bound_args
  // above (not ref-bumped), so skip past them without freeing -- same
  // "args have been transfered; don't free them" idiom as int_call_out().
  sp -= cb->narg;
  pop_stack();  // remove the callback; now sp = sql, sp-1 = handle

  array_t* info;
  info = allocate_empty_array(1);
  info->item[0].type = T_STRING;
  info->item[0].subtype = STRING_MALLOC;
  info->item[0].u.string = string_copy(sp->u.string, "f_db_exec");
  valid_database("exec", info);

  db_t* db;
  db = find_db_conn((sp - 1)->u.number);
  if (!db) {
    error("Attempt to exec on an invalid database handle\n");
  }

  if (!db_mut) {
    db_mut = (pthread_mutex_t*)DMALLOC(sizeof(pthread_mutex_t), TAG_PERMANENT, "async_db_exec");
    pthread_mutex_init(db_mut, nullptr);
  }
  handed_off = true;
  bound_args_handed_off = true;
  add_db_exec((sp - 1)->u.number, sp->u.string, cb.release(), bound_args);
  pop_2_elems();
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
    if (req->command_giver != nullptr) {
      req->command_giver->extra_ref++;
    }
    if (req->bound_args != nullptr) {
      req->bound_args->extra_ref++;
    }
  }
#endif
}
