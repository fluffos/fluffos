#include "base/package_api.h"

#include "packages/async/async.h"

#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
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

enum atypes { aread, awrite, agetdir, adbexec, done };

enum astates { BUSY, DONE };

struct request {
  char path[MAXPATHLEN];
  int flags;
  int ret;
  void *buf;
  int size;
  std::string sql;
  function_to_call_t *fun;
  struct request *next;
  svalue_t tmp;
  enum atypes type;
  int status;
};

struct work {
  struct request *data;
  void *(*func)(struct request *);
};

std::deque<struct work *> reqs;
std::mutex reqs_lock;

std::deque<struct request *> finished_reqs;
std::mutex finished_reqs_lock;

void thread_func() {
  Tracer::setThreadName("Package Async thread");

  ScopedTracer _tracer("Async thread loop");

  while (true) {
    struct work *w = nullptr;
    {
      std::lock_guard<std::mutex> _lock(reqs_lock);
      if (reqs.empty()) {
        return;
      }
      w = reqs.front();
      reqs.pop_front();
    }

    if (w) {
      {
        ScopedTracer _work_tracer("Async thread work", EventCategory::DEFAULT,
                                  json{{"type", w->data->type}});

        w->func(w->data);
      }
      if (w->data->status == DONE) {
        {
          std::lock_guard<std::mutex> _lock(finished_reqs_lock);
          finished_reqs.push_back(w->data);
        }
        delete w;
      } else {
        std::lock_guard<std::mutex> _lock(reqs_lock);
        reqs.push_back(w);
      }

      add_walltime_event(std::chrono::milliseconds(0),
                         tick_event::callback_type([] { check_reqs(); }));
    }
  }
}

void do_stuff(void *(*func)(struct request *), struct request *data) {
  std::lock_guard<std::mutex> _lock(reqs_lock);

  if (reqs.empty()) {
    std::thread(thread_func).detach();
  }

  auto i = new work;
  i->func = func;
  i->data = data;

  reqs.push_back(i);
}

void *gzreadthread(struct request *req) {
  gzFile file = gzopen(req->path, "rb");
  req->ret = gzread(file, (void *)(req->buf), req->size);
  req->status = DONE;
  gzclose(file);
  return nullptr;
}

int aio_gzread(struct request *req) {
  req->status = BUSY;
  do_stuff(gzreadthread, req);
  return 0;
}

void *gzwritethread(struct request *req) {
  int fd =
      open(req->path, req->flags & 1 ? O_CREAT | O_WRONLY | O_TRUNC : O_CREAT | O_WRONLY | O_APPEND,
           S_IRWXU | S_IRWXG);
  gzFile file = gzdopen(fd, "wb");
  req->ret = gzwrite(file, (void *)(req->buf), req->size);
  req->status = DONE;
  gzclose(file);
  return nullptr;
}

int aio_gzwrite(struct request *req) {
  req->status = BUSY;
  do_stuff(gzwritethread, req);
  return 0;
}

void *writethread(struct request *req) {
  int fd =
      open(req->path, req->flags & 1 ? O_CREAT | O_WRONLY | O_TRUNC : O_CREAT | O_WRONLY | O_APPEND,
           S_IRWXU | S_IRWXG);

  req->ret = write(fd, req->buf, req->size);

  req->status = DONE;
  close(fd);
  return nullptr;
}

int aio_write(struct request *req) {
  req->status = BUSY;
  do_stuff(writethread, req);
  return 0;
}

void *readthread(struct request *req) {
  int fd = open(req->path, O_RDONLY);
  req->ret = read(fd, (void *)(req->buf), req->size);
  req->status = DONE;
  close(fd);
  return nullptr;
}

int aio_read(struct request *req) {
  req->status = BUSY;
  do_stuff(readthread, req);
  return 0;
}

#ifdef F_ASYNC_DB_EXEC
pthread_mutex_t *db_mut = nullptr;

void *dbexecthread(struct request *req) {
  ScopedTracer _work_tracer("db_exec", EventCategory::DEFAULT, json{req->sql});

  pthread_mutex_lock(db_mut);
  // see add_db_exec
  db_t *db = find_db_conn((intptr_t)(req->buf));
  int ret = -1;
  if (db && db->type->execute) {
    if (db->type->cleanup) {
      db->type->cleanup(&(db->c));
    }

    ret = db->type->execute(&(db->c), req->sql.c_str());
    if (ret == -1) {
      if (db->type->error) {
        char *tmp;
        strncpy(req->path, tmp = db->type->error(&(db->c)), MAXPATHLEN - 1);
        FREE_MSTR(tmp);
      } else {
        strcpy(req->path, "Unknown error");
      }
    }
  } else {
    strcpy(req->path, "No database exec function!");
  }
  pthread_mutex_unlock(db_mut);

  req->ret = ret;
  req->status = DONE;
  return nullptr;
}

int aio_db_exec(struct request *req) {
  req->status = BUSY;
  do_stuff(dbexecthread, req);
  return 0;
}
#endif

#ifdef F_ASYNC_GETDIR
void *getdirthread(struct request *req) {
  ScopedTracer _work_tracer("getdir", EventCategory::DEFAULT, json{req->path});

  DIR *dirp = nullptr;
  if ((dirp = opendir(req->path)) == nullptr) {
    req->ret = 0;
    req->status = DONE;
    return nullptr;
  }
  /*
   * Count files
   */
  int i = 0;
  for (auto de = readdir(dirp); de; de = readdir(dirp)) {
    if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) continue;
    memcpy(&((dirent *)(req->buf))[i], de, sizeof(*de));
    i++;
  }

  closedir(dirp);

  req->ret = i;
  req->status = DONE;
  return nullptr;
}

int aio_getdir(struct request *req) {
  req->status = BUSY;
  do_stuff(getdirthread, req);
  return 0;
}

#endif

int add_read(const char *fname, function_to_call_t *fun) {
  const auto read_file_max_size = CONFIG_INT(__MAX_READ_FILE_SIZE__);

  if (fname) {
    auto *req = new request();
    // printf("fname: %s\n", fname);
    req->buf = reinterpret_cast<char *>(DMALLOC(read_file_max_size, TAG_PERMANENT, "add_read"));
    req->size = read_file_max_size;
    req->fun = fun;
    req->type = aread;
    strcpy(req->path, fname);
    return aio_gzread(req);
  } else {
    error("permission denied\n");
  }
  return 1;
}

#ifdef F_ASYNC_GETDIR
int add_getdir(const char *fname, function_to_call_t *fun) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  if (fname) {
    // printf("fname: %s\n", fname);
    auto *req = new request();
    req->buf = DMALLOC(sizeof(struct dirent) * max_array_size, TAG_PERMANENT, "add_getdir");
    req->size = max_array_size;
    req->fun = fun;
    req->type = agetdir;
    strcpy(req->path, fname);
    return aio_getdir(req);
  } else {
    error("permission denied\n");
  }
  return 1;
}
#endif

int add_write(const char *fname, const char *buf, int size, char flags, function_to_call_t *fun) {
  if (fname) {
    auto *req = new request();
    req->buf = (void *)buf;
    req->size = size;
    req->fun = fun;
    req->type = awrite;
    req->flags = flags;
    strcpy(req->path, fname);
    assign_svalue_no_free(&req->tmp, sp - 1);
    if (flags & 2) {
      return aio_gzwrite(req);
    } else {
      return aio_write(req);
    }
  } else {
    error("permission denied\n");
  }
  return 1;
}

#ifdef F_ASYNC_DB_EXEC
int add_db_exec(int handle, const char *sql, function_to_call_t *fun) {
  auto *req = new request();
  req->fun = fun;
  req->type = adbexec;
  req->buf = reinterpret_cast<void *>((intptr_t)(handle));
  req->sql = sql;
  return aio_db_exec(req);
}
#endif

void handle_read(struct request *req) {
  int val = req->ret;
  if (val < 0) {
    FREE((void *)req->buf);
    push_number(val);
    set_eval(max_eval_cost);
    safe_call_efun_callback(req->fun, 1);
    return;
  }
  char *file = new_string(val, "read_file_async: str");
  memcpy(file, (char *)(req->buf), val);
  file[val] = 0;
  push_malloced_string(file);
  FREE((void *)req->buf);
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}

#ifdef F_ASYNC_GETDIR

void handle_getdir(struct request *req) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  int ret_size = req->ret;
  if (ret_size > max_array_size) {
    ret_size = max_array_size;
  }
  array_t *ret = allocate_empty_array(ret_size);
  if (ret_size > 0) {
    for (int i = 0; i < ret_size; i++) {
      auto de = ((struct dirent *)req->buf)[i];
      svalue_t *vp = &(ret->item[i]);
      vp->type = T_STRING;
      vp->subtype = STRING_MALLOC;
      vp->u.string = string_copy(de.d_name, "encode_stat");
    }

    qsort((void *)ret->item, ret_size, sizeof ret->item[0],
          [](const void *p1, const void *p2) -> int {
            auto *x = (svalue_t *)p1;
            auto *y = (svalue_t *)p2;

            return strcmp(x->u.string, y->u.string);
          });
  }

  FREE((void *)req->buf);

  push_refed_array(ret);
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}
#endif

void handle_write(struct request *req) {
  free_svalue(&req->tmp, "handle_write");
  int val = req->ret;
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

void handle_db_exec(struct request *req) {
  free_svalue(&req->tmp, "handle_db_exec");
  int val = req->ret;
  if (val == -1) {
    copy_and_push_string(req->path);
  } else {
    push_number(val);
  }
  set_eval(max_eval_cost);
  safe_call_efun_callback(req->fun, 1);
}

void check_reqs() {
  ScopedTracer _tracer("Async callback");

  std::lock_guard<std::mutex> _lock(finished_reqs_lock);
  while (!finished_reqs.empty()) {
    auto req = finished_reqs.front();
    finished_reqs.pop_front();

    enum atypes type = (req->type);
    req->type = done;
    switch (type) {
      case aread:
        handle_read(req);
        break;
      case awrite:
        handle_write(req);
        break;
#ifdef F_ASYNC_GETDIR
      case agetdir:
        handle_getdir(req);
        break;
#endif
#ifdef F_ASYNC_DB_EXEC
      case adbexec:
        handle_db_exec(req);
        break;
#endif
      case done:
        // must have had an error while handling it before.
        break;
      default:
        fatal("unknown async type\n");
    }
#ifdef DEBUGMALLOC_EXTENSIONS
    req->fun->f.fp->hdr.extra_ref--;
#endif
    free_funp(req->fun->f.fp);
    delete req->fun;
    delete req;
  }
}

void complete_all_asyncio() {
  while (true) {
    std::lock_guard<std::mutex> _lock(reqs_lock);

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
  cb->f.fp->hdr.ref++;
#ifdef DEBUGMALLOC_EXTENSIONS
  cb->f.fp->hdr.extra_ref++;
#endif
  pop_stack();

  add_read(check_valid_path(sp->u.string, current_object, "read_file", 0), cb.release());
  pop_stack();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(3, cb.get(), F_ASYNC_WRITE);
  cb->f.fp->hdr.ref++;
#ifdef DEBUGMALLOC_EXTENSIONS
  cb->f.fp->hdr.extra_ref++;
#endif
  pop_stack();

  add_write(check_valid_path((sp - 2)->u.string, current_object, "write_file", 1),
            (sp - 1)->u.string, strlen((sp - 1)->u.string), sp->u.number, cb.release());
  pop_3_elems();
}
#endif

#ifdef F_ASYNC_GETDIR
void f_async_getdir() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(1, cb.get(), F_ASYNC_GETDIR);
  cb->f.fp->hdr.ref++;
#ifdef DEBUGMALLOC_EXTENSIONS
  cb->f.fp->hdr.extra_ref++;
#endif
  pop_stack();

  add_getdir(check_valid_path(sp->u.string, current_object, "get_dir", 0), cb.release());
  pop_stack();
}
#endif
#ifdef F_ASYNC_DB_EXEC
void f_async_db_exec() {
  std::unique_ptr<function_to_call_t> cb(new function_to_call_t);
  process_efun_callback(2, cb.get(), F_ASYNC_DB_EXEC);
  cb->f.fp->hdr.ref++;
#ifdef DEBUGMALLOC_EXTENSIONS
  cb->f.fp->hdr.extra_ref++;
#endif
  pop_stack();

  array_t *info;
  info = allocate_empty_array(1);
  info->item[0].type = T_STRING;
  info->item[0].subtype = STRING_MALLOC;
  info->item[0].u.string = string_copy(sp->u.string, "f_db_exec");
  valid_database("exec", info);

  db_t *db;
  db = find_db_conn((sp - 1)->u.number);
  if (!db) {
    error("Attempt to exec on an invalid database handle\n");
  }

  if (!db_mut) {
    db_mut = (pthread_mutex_t *)DMALLOC(sizeof(pthread_mutex_t), TAG_PERMANENT, "async_db_exec");
    pthread_mutex_init(db_mut, nullptr);
  }
  add_db_exec((sp - 1)->u.number, sp->u.string, cb.release());
  pop_2_elems();
}
#endif
