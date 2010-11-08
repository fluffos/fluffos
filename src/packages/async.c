#include "std.h"
#include "../lpc_incl.h"
#include "async.h"
#include "../function.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#ifdef F_ASYNC_GETDIR
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/syscall.h>
#endif
#include "../config.h"
#include "../interpret.h"
#include "../file.h"
#include "../function.h"
#include "../eval.h"
#ifdef F_ASYNC_DB_EXEC
#include "db.h"
#endif

enum atypes {
	aread,
	awrite,
	agetdir,
	adbexec,
	done
};

enum astates {
	BUSY,
	DONE,
};

struct request{
	char path[MAXPATHLEN];
	int flags;
	int status;
	int ret;
	const char *buf;
	int size;
	function_to_call_t *fun;
	struct request *next;
	svalue_t tmp;
	enum atypes type;
};

void add_req(struct request *req);

#if defined(F_ASYNC_READ) || defined(F_ASYNC_WRITE)

struct cb_mem{
	function_to_call_t cb;
	struct cb_mem *next;
} *cbs = 0;

struct req_mem{
	struct request req;
	struct req_mem *next;
} *reqms;

struct stuff{
	void *(*func)(struct request *);
	struct request *data;
	struct stuff *next;
} *todo, *lasttodo;

struct stuff_mem{
	struct stuff stuff;
	struct stuff_mem *next;
} *stuffs;

pthread_mutex_t mem_mut;


struct stuff *get_stuff(){
	struct stuff *ret;
	if(stuffs){
		pthread_mutex_lock(&mem_mut);
		ret = &stuffs->stuff;
		stuffs = stuffs->next;
		((struct stuff_mem *)ret)->next = 0;
		pthread_mutex_unlock(&mem_mut);
	}else{
		ret = (struct stuff *)MALLOC(sizeof(struct stuff_mem));
		((struct stuff_mem *)ret)->next = 0;
	}
	return ret;
}

void free_stuff(struct stuff *stuff){
	struct stuff_mem *stufft = (struct stuff_mem *)stuff;
	pthread_mutex_lock(&mem_mut);
	stufft->next = stuffs;
	stuffs = stufft;
	pthread_mutex_unlock(&mem_mut);
}

pthread_mutex_t mut;
pthread_mutex_t work_mut;
int thread_started = 0;

void *thread_func(void *mydata){
	while(1){
		pthread_mutex_lock(&mut);
		while(todo){
			pthread_mutex_lock(&work_mut);
			struct stuff *work = todo;
			todo = todo->next;
			if(!todo)
				lasttodo = NULL;
			pthread_mutex_unlock(&work_mut);
			work->func(work->data);
			free_stuff(work);
		}
	}
}

void do_stuff(void *(*func)(struct request *), struct request *data){
	if(!thread_started){
		pthread_mutex_init(&mut, NULL);
		pthread_mutex_init(&mem_mut, NULL);
		pthread_mutex_init(&work_mut, NULL);
		pthread_mutex_lock(&mut);
		pthread_t t;
		pthread_create(&t, NULL, &thread_func, NULL);
		thread_started = 1;
	}
	struct stuff *work = get_stuff();
	work->func = func;
	work->data = data;
	work->next = NULL;
	pthread_mutex_lock(&work_mut);
	add_req(data);
	if(lasttodo){
		lasttodo->next = work;
		lasttodo = work;
	} else {
		todo = lasttodo = work;
	}
	pthread_mutex_unlock(&work_mut);
	pthread_mutex_unlock(&mut);
}

function_to_call_t *get_cb(){
	function_to_call_t *ret;
	if(cbs){
		ret = &cbs->cb;
		cbs = cbs->next;
		((struct cb_mem *)ret)->next = 0;
	}else{
		ret = (function_to_call_t *)MALLOC(sizeof(struct cb_mem));
		((struct cb_mem *)ret)->next = 0;
	}
	memset(ret, 0, sizeof(function_to_call_t));
	return ret;
}

void free_cb(function_to_call_t *cb){
	struct cb_mem *cbt = (struct cb_mem *)cb;
	cbt->next = cbs;
	cbs = cbt;
}

struct request *get_req(){
	struct request *ret;
	if(reqms){
		ret = &reqms->req;
		reqms = reqms->next;
		((struct req_mem *)ret)->next = 0;
	}else{
		ret = (struct request *)MALLOC(sizeof(struct req_mem));
		((struct req_mem *)ret)->next = 0;
	}
	return ret;
}

void free_req(struct request *req){
	struct req_mem *reqt = (struct req_mem *)req;
	reqt->next = reqms;
	reqms = reqt;
}


static struct request *reqs = NULL;
static struct request *lastreq = NULL;
void add_req(struct request *req){
	if(lastreq){
		lastreq->next = req;
	} else {
		reqs = req;
	}
	req->next = NULL;
	lastreq = req;
}

#ifdef PACKAGE_COMPRESS
#include <zlib.h>

void *gzreadthread(struct request *req){
	void *file = gzopen(req->path, "rb");
	req->ret = gzread(file, (void *)(req->buf), req->size);
	req->status = DONE;
	gzclose(file);
	return NULL;
}

int aio_gzread(struct request *req){
	req->status = BUSY;
	do_stuff(gzreadthread, req);
	return 0;
}

void *gzwritethread(struct request *req){
	int fd = open(req->path, req->flags & 1 ? O_CREAT|O_WRONLY|O_TRUNC
			: O_CREAT|O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG);
	void *file = gzdopen(fd, "wb");
	req->ret = gzwrite(file, (void *)(req->buf), req->size);
	req->status = DONE;
	gzclose(file);
	return NULL;
}

int aio_gzwrite(struct request *req){
	req->status = BUSY;
	do_stuff(gzwritethread, req);
	return 0;
}
#endif

void *writethread(struct request *req){
	int fd = open(req->path, req->flags & 1 ? O_CREAT|O_WRONLY|O_TRUNC
			: O_CREAT|O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG);

	req->ret =  write(fd, req->buf, req->size);

	req->status = DONE;
	close(fd);
	return NULL;
}

int aio_write(struct request *req){
	req->status = BUSY;
	do_stuff(writethread, req);
	return 0;
}

void *readthread(struct request *req){
	int fd = open(req->path, O_RDONLY);
	req->ret = read(fd, (void *)(req->buf), req->size);
	req->status = DONE;
	close(fd);
	return NULL;
}

int aio_read(struct request *req){
	req->status = BUSY;
	do_stuff(readthread, req);
	return 0;
}

#ifdef F_ASYNC_DB_EXEC
pthread_mutex_t *db_mut = NULL;

void *dbexecthread(struct request *req){
	pthread_mutex_lock(db_mut);
	db_t *db = find_db_conn((int)req->buf);
	int ret = -1;
	if (db->type->execute) {
		if (db->type->cleanup) {
			db->type->cleanup(&(db->c));
		}

		ret = db->type->execute(&(db->c), req->tmp.u.string);
		if (ret == -1){
			if(db->type->error) {
				char *tmp;
				strncpy(req->path, tmp = db->type->error(&(db->c)), MAXPATHLEN-1);
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
	return NULL;
}

int aio_db_exec(struct request *req){
	req->status = BUSY;
	do_stuff(dbexecthread, req);
	return 0;
}
#endif

#ifdef F_ASYNC_GETDIR
void *getdirthread(struct request *req){
	int fd = open(req->path, O_RDONLY);
	int size = syscall(SYS_getdents, fd, req->buf, req->size);
	if(size == -1){
		close(fd);
		req->ret = 0;
		req->status = DONE;
		return NULL;
	}
	req->ret = size;
	while(size = syscall(SYS_getdents, fd, req->buf+req->ret, req->size-req->ret)){
		if(size == -1){
			close(fd);
			req->status = DONE;
			return NULL;
		}
		req->ret+=size;
	}
	req->status = DONE;
	close(fd);
	return NULL;
}

int aio_getdir(struct request *req){
	req->status = BUSY;
	do_stuff(getdirthread, req);
	return 0;
}

#endif

int add_read(const char *fname, function_to_call_t *fun) {
	if (fname) {
		struct request *req = get_req();
		//printf("fname: %s\n", fname);
		req->buf = (char *)MALLOC(READ_FILE_MAX_SIZE);
		req->size = READ_FILE_MAX_SIZE;
		req->fun = fun;
		req->type = aread;
		strcpy(req->path, fname);
#ifdef PACKAGE_COMPRESS
		return aio_gzread(req);
#else
		return aio_read(req);
#endif
	}else
		error("permission denied\n");
	return 1;
}

#ifdef F_ASYNC_GETDIR
extern int max_array_size;
int add_getdir(const char *fname, function_to_call_t *fun) {
	if (fname) {
		//printf("fname: %s\n", fname);
		struct request *req = get_req();
		req->buf = (char *)MALLOC(sizeof(struct dirent) * max_array_size);
		req->size = sizeof(struct dirent) * max_array_size;
		req->fun = fun;
		req->type = agetdir;
		strcpy(req->path, fname);
		return aio_getdir(req);
	}else
		error("permission denied\n");
	return 1;
}
#endif

int add_write(const char *fname, const char *buf, int size, char flags, function_to_call_t *fun) {
	if (fname) {
		struct request *req = get_req();
		req->buf = buf;
		req->size = size;
		req->fun = fun;
		req->type = awrite;
		req->flags = flags;
		strcpy(req->path, fname);
		assign_svalue_no_free(&req->tmp, sp-2);
#ifdef PACKAGE_COMPRESS
		if(flags & 2)
			return aio_gzwrite(req);
		else
#endif
			return aio_write(req);
	} else
		error("permission denied\n");
	return 1;
}

#ifdef F_ASYNC_DB_EXEC
int add_db_exec(int handle, function_to_call_t *fun) {
	struct request *req = get_req();
	req->fun = fun;
	req->type = adbexec;
	req->buf = (char *)handle;
	assign_svalue_no_free(&req->tmp, sp-1);
	return aio_db_exec(req);
}
#endif


void handle_read(struct request *req){
	int val = req->ret;
	if(val < 0){
		FREE((void *)req->buf);
		push_number(val);
		set_eval(max_cost);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	char *file = new_string(val, "read_file_async: str");
	memcpy(file, (char *)(req->buf), val);
	file[val]=0;
	push_malloced_string(file);
	FREE((void *)req->buf);
	set_eval(max_cost);
	safe_call_efun_callback(req->fun, 1);
}

#ifdef F_ASYNC_GETDIR

struct linux_dirent {
	unsigned long  d_ino;     /* Inode number */
	unsigned long  d_off;     /* Offset to next dirent */
	unsigned short d_reclen;  /* Length of this dirent */
	char           d_name []; /* Filename (null-terminated) */
	/* length is actually (d_reclen - 2 -
                                      offsetof(struct linux_dirent, d_name) */
};


void handle_getdir(struct request *req){
	int val = req->ret;
	if(val>MAX_ARRAY_SIZE)
		val = MAX_ARRAY_SIZE;
	array_t *ret = allocate_empty_array(val);
	int i=0;
	if(val > 0)
	{
		struct linux_dirent *de = (struct linux_dirent *)req->buf;
		for(i=0; i<MAX_ARRAY_SIZE && ((char *)de) - (char *)(req->buf) < val; i++)
		{
			svalue_t *vp = &(ret->item[i]);
			vp->type = T_STRING;
			vp->subtype = STRING_MALLOC;
			vp->u.string = string_copy(de->d_name, "encode_stat");
			de = (struct linux_dirent *)(((char *)de) + de->d_reclen);
		}
	}
	ret = resize_array(ret, i);
	ret->size = i;
	push_refed_array(ret);
	FREE((void *)req->buf);
	set_eval(max_cost);
	safe_call_efun_callback(req->fun, 1);
}
#endif


void handle_write(struct request *req){
	free_svalue(&req->tmp, "handle_write");
	int val = req->ret;
	if(val < 0){
		push_number(val);
		set_eval(max_cost);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	push_undefined();
	set_eval(max_cost);
	safe_call_efun_callback(req->fun, 1);
}

void handle_db_exec(struct request *req){
	free_svalue(&req->tmp, "handle_db_exec");
	int val = req->ret;
	if(val == -1){
		copy_and_push_string(req->path);
	}
	else
		push_number(val);
	set_eval(max_cost);
	safe_call_efun_callback(req->fun, 1);
}

void check_reqs() {
	while (reqs) {
		int val = reqs->status;
		if (val != BUSY) {
			enum atypes type =  (reqs->type);
			reqs->type = done;
			switch (type) {
			case aread:
				handle_read(reqs);
				break;
			case awrite:
				handle_write(reqs);
				break;
#ifdef F_ASYNC_GETDIR
			case agetdir:
				handle_getdir(reqs);
				break;
#endif
#ifdef F_ASYNC_DB_EXEC
			case adbexec:
				handle_db_exec(reqs);
				break;
#endif
			case done:
				//must have had an error while handling it before.
				break;
			default:
				fatal("unknown async type\n");
			}
			struct request *here = reqs;
			reqs = reqs->next;
			if(!reqs)
				lastreq = reqs;
			free_funp(here->fun->f.fp);
			free_cb(here->fun);
			free_req(here);
		} else
			return;
	}
}

void complete_all_asyncio(){
	while(reqs)
		check_reqs();
}

#ifdef F_ASYNC_READ

void f_async_read(){
	function_to_call_t *cb = get_cb();
	process_efun_callback(1, cb, F_ASYNC_READ);
	cb->f.fp->hdr.ref++;
	add_read(check_valid_path((sp-1)->u.string, current_object, "read_file", 0), cb);
	pop_2_elems();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write(){
	function_to_call_t *cb = get_cb();
	process_efun_callback(3, cb, F_ASYNC_WRITE);
	cb->f.fp->hdr.ref++;
	add_write(check_valid_path((sp-3)->u.string, current_object, "write_file", 1), (sp-2)->u.string, strlen((sp-2)->u.string), (sp-1)->u.number, cb);
	pop_n_elems(4);
}
#endif

#ifdef F_ASYNC_GETDIR
void f_async_getdir(){
	function_to_call_t *cb = get_cb();
	process_efun_callback(1, cb, F_ASYNC_READ);
	cb->f.fp->hdr.ref++;
	add_getdir(check_valid_path((sp-1)->u.string, current_object, "get_dir", 0), cb);
	pop_2_elems();
}
#endif
#ifdef F_ASYNC_DB_EXEC
void f_async_db_exec(){
	array_t *info;
	db_t *db;
	info = allocate_empty_array(1);
	info->item[0].type = T_STRING;
	info->item[0].subtype = STRING_MALLOC;
	info->item[0].u.string = string_copy((sp-1)->u.string, "f_db_exec");
	int num_arg = st_num_arg;
	valid_database("exec", info);

	db = find_db_conn((sp-2)->u.number);
	if (!db) {
		error("Attempt to exec on an invalid database handle\n");
	}
	if(!db_mut){
		db_mut = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(db_mut, NULL);
	}
	st_num_arg = num_arg;
	function_to_call_t *cb = get_cb();
	process_efun_callback(2, cb, F_ASYNC_DB_EXEC);
	cb->f.fp->hdr.ref++;

	add_db_exec((sp-2)->u.number, cb);
	pop_3_elems();
}
#endif
#endif
