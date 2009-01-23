#include "std.h"
#include "../lpc_incl.h"
#include "async.h"
#include "../function.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define _GNU_SOURCE
#include <sys/syscall.h>
#include "../config.h"
#include "../interpret.h"
#include "../file.h"
#if defined(F_ASYNC_READ) || defined(F_ASYNC_WRITE)

static struct request *reqs = NULL;
void add_req(struct request *req){
	req->next = reqs;
	reqs = req;
}

#if defined PACKAGE_COMPRESS && defined linux
#include <pthread.h>
#include <zlib.h>

void *gzreadthread(void *data){
    aiob *aio = (aiob *)data;
	void *file = gzdopen(dup(aio->aio_fildes), "rb");
	aio->__return_value = gzread(file, (void *)(aio->aio_buf), aio->aio_nbytes);
	aio->__error_code = 0;
	gzclose(file);
	return NULL;
}

int aio_gzread(aiob *aio){
    pthread_t thread;
	aio->__error_code = EINPROGRESS;
	pthread_create(&thread, NULL, gzreadthread, aio);
	return 0;
}

void *gzwritethread(void *data){
    aiob *aio = (aiob *)data;
	void *file = gzdopen(dup(aio->aio_fildes), "wb");
	aio->__return_value = gzwrite(file, (void *)(aio->aio_buf), aio->aio_nbytes);
	aio->__error_code = 0;
	gzclose(file);
	return NULL;
}

int aio_gzwrite(aiob *aio){
    pthread_t thread;
	aio->__error_code = EINPROGRESS;
	pthread_create(&thread, NULL, gzwritethread, aio);
	return 0;
}
#endif

#ifdef F_ASYNC_GETDIR
void *getdirthread(void *data){
    aiob *aio = (aiob *)data;
    aio->__return_value = syscall(SYS_getdents, aio->aio_fildes, aio->aio_buf, aio->aio_nbytes);
	aio->__error_code = 0;
	return NULL;
}

int aio_getdir(aiob *aio){
    pthread_t *thread = (pthread_t *)MALLOC(sizeof(pthread_t));
	aio->__error_code = EINPROGRESS;
	pthread_create(thread, NULL, getdirthread, aio);
	FREE(thread); //like WE care
	return 0;
}

#endif

int add_read(const char *fname, function_to_call_t *fun) {
	if (fname) {
	        aiob *aio= (aiob *)MALLOC(sizeof(aiob));
		memset(aio, 0, sizeof(aiob));
		//printf("fname: %s\n", fname);
		int fd = open(fname, O_RDONLY);
		aio->aio_fildes = fd;
		aio->aio_buf = (char *)MALLOC(READ_FILE_MAX_SIZE);
		aio->aio_nbytes = READ_FILE_MAX_SIZE;
		struct request *req = (struct request *)MALLOC(sizeof(struct request));
		req->aio = aio;
		req->fun = fun;
		req->type = aread;
		add_req(req);
#if defined PACKAGE_COMPRESS && defined linux
		return aio_gzread(aio);
#else
		return aio_read(aio);
#endif
	}
	return 1;
}

#ifdef F_ASYNC_GETDIR
extern int max_array_size;
int add_getdir(const char *fname, function_to_call_t *fun) {
	if (fname) {
	    aiob *aio= (aiob *)MALLOC(sizeof(aiob));
		memset(aio, 0, sizeof(aiob));
		//printf("fname: %s\n", fname);
		int fd = open(fname, O_RDONLY);
		aio->aio_fildes = fd;
		aio->aio_buf = (char *)MALLOC(sizeof(struct dirent) * max_array_size);
		aio->aio_nbytes = sizeof(struct dirent) * max_array_size;
		struct request *req = (struct request *)MALLOC(sizeof(struct request));
		req->aio = aio;
		req->fun = fun;
		req->type = agetdir;
		add_req(req);
		return aio_getdir(aio);
	}
	return 1;
}
#endif

int add_write(const char *fname, char *buf, int size, char flags, function_to_call_t *fun) {
	if (fname) {
	    aiob *aio = (aiob *)MALLOC(sizeof(aiob));
		memset(aio, 0, sizeof(aiob));
		int fd = open(fname, flags & 1 ? O_CREAT|O_WRONLY
				: O_CREAT|O_WRONLY|O_APPEND, S_IRWXU|S_IRWXG);
		aio->aio_fildes = fd;
		aio->aio_buf = buf;
		aio->aio_nbytes = size;
		struct request *req = (struct request *)MALLOC(sizeof(struct request));
		req->aio = aio;
		req->fun = fun;
		req->type = awrite;
		add_req(req);
#if defined PACKAGE_COMPRESS && defined linux
		if(flags & 2)
			return aio_gzwrite(aio);
		else
#endif
			return aio_write(aio);
	}
	FREE(buf);
	return 1;
}

void handle_read(struct request *req, int val){
	aiob *aio = req->aio;
	close(aio->aio_fildes);
	if(val){
		push_number(val);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	val = aio_return(aio);
	if(val < 0){
		push_number(val);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	char *file = new_string(val, "read_file_async: str");
	memcpy(file, (char *)(aio->aio_buf), val);
	file[val]=0;
	push_malloced_string(file);
	safe_call_efun_callback(req->fun, 1);
}

#ifdef F_ASYNC_GETDIR
void handle_getdir(struct request *req, int val){
	aiob *aio = req->aio;
	close(aio->aio_fildes);
	val = aio_return(aio);
	array_t *ret = allocate_empty_array(val);
	int i;
	if(val > -1)
	{
		struct dirent *de = (struct dirent *)aio->aio_buf;
		for(i=0; ((char *)de) - (char *)(aio->aio_buf) < val; i++)
		{
			svalue_t *vp = &(ret->item[i]);
			vp->type = T_STRING;
			vp->subtype = STRING_MALLOC;
			//printf("%s ", de->d_name);
			vp->u.string = string_copy((de->d_name - 1), "encode_stat"); //hmm, wrong struct??
			de = (struct dirent *)(((char *)de) + de->d_reclen);
		}
	}
    ret = RESIZE_ARRAY(ret, i);
    ret->size = i;
	push_refed_array(ret);
	safe_call_efun_callback(req->fun, 1);
}
#endif


void handle_write(struct request *req, int val){
	aiob *aio = req->aio;
	close(aio->aio_fildes);
	if(val){
		push_number(val);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	val = aio_return(aio);
	if(val < 0){
		push_number(val);
		safe_call_efun_callback(req->fun, 1);
		return;
	}
	push_undefined();
	safe_call_efun_callback(req->fun, 1);
}

void check_reqs() {
	struct request **check = &reqs;
	while (*check) {
		struct request *here = *check;
		int val = aio_error((*check)->aio);
		if (val != EINPROGRESS) {
			enum atypes type =  ((*check)->type);
			(*check)->type = done;
			switch (type) {
			case aread:
				handle_read(here, val);
				break;
			case awrite:
				handle_write(here, val);
				break;
#ifdef F_ASYNC_GETDIR
			case agetdir:
				handle_getdir(here, val);
				break;
#endif
			case done:
				//must have had an error while handling it before.
				break;
			default:
				fatal("unknown async type\n");
			}
			while(*check != here)
				check = &(*check)->next;
			*check = (*check)->next;
			FREE((char *)(here->aio->aio_buf));
			FREE(here->aio);
			free_funp(here->fun->f.fp);
			FREE(here->fun);
			FREE(here);
		} else {
			struct request *tmp = *check;
			if(tmp->next)
				check = &((*check)->next);
			else
				return;
		}
	}
}

#ifdef F_ASYNC_READ

void f_async_read(){
    function_to_call_t *cb = (function_to_call_t *)MALLOC(sizeof(function_to_call_t));
	memset(cb, 0, sizeof(function_to_call_t));
	process_efun_callback(1, cb, F_ASYNC_READ);
	cb->f.fp->hdr.ref++;
	add_read(check_valid_path((sp-1)->u.string, current_object, "read_file", 0), cb);
	pop_2_elems();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write(){
    char *buf = (char *)MALLOC(strlen((sp-2)->u.string)+1);
	strcpy(buf, (sp-2)->u.string);
	function_to_call_t *cb = (function_to_call_t *)MALLOC(sizeof(function_to_call_t));
	memset(cb, 0, sizeof(function_to_call_t));
	process_efun_callback(3, cb, F_ASYNC_WRITE);
	cb->f.fp->hdr.ref++;
	add_write(check_valid_path((sp-3)->u.string, current_object, "write_file", 1), buf, strlen(buf), (sp-1)->u.number, cb);
	pop_n_elems(4);
}
#endif

#ifdef F_ASYNC_GETDIR
void f_async_getdir(){
    function_to_call_t *cb = (function_to_call_t *)MALLOC(sizeof(function_to_call_t));
	memset(cb, 0, sizeof(function_to_call_t));
	process_efun_callback(1, cb, F_ASYNC_READ);
	cb->f.fp->hdr.ref++;
	add_getdir(check_valid_path((sp-1)->u.string, current_object, "get_dir", 0), cb);
	pop_2_elems();
}
#endif

#endif
