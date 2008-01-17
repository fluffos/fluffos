#include "std.h"
#include "../lpc_incl.h"
#include "async.h"
#include "../function.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../config.h"
#include "../interpret.h"
#include "../file.h"
#if defined(F_ASYNC_READ) || defined(F_ASYNC_WRITE)

static struct request *reqs = NULL;
void add_req(struct request *req){
	req->next = reqs;
	reqs = req;
}

int add_read(const char *fname, function_to_call_t *fun){
	aiob *aio = MALLOC(sizeof(aiob));
	memset(aio, 0, sizeof(aiob));
	printf("fname: %s\n", fname);
	int fd = open(fname, O_RDONLY);
	aio->aio_fildes = fd;
	aio->aio_buf = MALLOC(READ_FILE_MAX_SIZE);
	aio->aio_nbytes = READ_FILE_MAX_SIZE;
	struct request *req = MALLOC(sizeof(struct request));
	req->aio = aio;
	req->fun = fun;
	req->type = aread;
	add_req(req);
	return aio_read(aio);
}

int add_write(const char *fname, char *buf, int size, char append, function_to_call_t *fun){
	aiob *aio = MALLOC(sizeof(aiob));
	memset(aio, 0, sizeof(aiob));
	int fd = open(fname, append?O_CREAT|O_WRONLY|O_APPEND:O_CREAT|O_WRONLY, S_IRWXU|S_IRWXG);
	aio->aio_fildes = fd;
	aio->aio_buf = buf;
	aio->aio_nbytes = size;
	struct request *req = MALLOC(sizeof(struct request));
	req->aio = aio;
	req->fun = fun;
	req->type = awrite;
	add_req(req);
	return aio_write(aio);
}

void handle_read(struct request *req, int val){
	aiob *aio = req->aio;
	close(aio->aio_fildes);
	if(val){
		push_number(val);
		call_efun_callback(req->fun, 1);
		free_funp(req->fun->f.fp);
		return;
	}
	val = aio_return(aio);
	if(val < 0){
		push_number(val);
		call_efun_callback(req->fun, 1);
		free_funp(req->fun->f.fp);
		return;
	}
	char *file = MALLOC(val+1);
	memcpy(file, (char *)(aio->aio_buf), val);
	file[val]=0;
	push_malloced_string(file);
	call_efun_callback(req->fun, 1);
	free_funp(req->fun->f.fp);
}

void handle_write(struct request *req, int val){
	aiob *aio = req->aio;
	close(aio->aio_fildes);
	if(val){
		push_number(val);
		call_efun_callback(req->fun, 1);
		free_funp(req->fun->f.fp);
		return;
	}
	val = aio_return(aio);
	if(val < 0){
		push_number(val);
		call_efun_callback(req->fun, 1);
		free_funp(req->fun->f.fp);
		return;
	}
	push_undefined();
	call_efun_callback(req->fun, 1);
	free_funp(req->fun->f.fp);
}

void check_reqs() {
	struct request **check = &reqs;
	while (*check) {
		int val = aio_error((*check)->aio);
		if (val != EINPROGRESS) {
			switch ((*check)->type) {
			case aread:
				handle_read(*check, val);
				break;
			case awrite:
				handle_write(*check, val);
				break;
			default:
				fatal("unknown async type\n");
			}
			struct request *tmp = *check;
			*check = (*check)->next;
			fflush(0);
			FREE((char *)(tmp->aio->aio_buf));
			FREE(tmp->aio);
			FREE(tmp->fun);
			FREE(tmp);
		} else {
			struct request *tmp = *check;
			if(tmp->next){
				fflush(0);
				check = &((*check)->next);
			} else
				check = 0;
		}
	}
}

#ifdef F_ASYNC_READ

void f_async_read(){
	function_to_call_t *cb = MALLOC(sizeof(function_to_call_t));
	memset(cb, 0, sizeof(function_to_call_t));
	process_efun_callback(1, cb, F_ASYNC_READ);
	cb->f.fp->hdr.ref++;
	add_read(check_valid_path((sp-1)->u.string, current_object, "read_file", 0), cb);
	pop_2_elems();
}
#endif

#ifdef F_ASYNC_WRITE
void f_async_write(){
	char *buf = MALLOC(strlen((sp-1)->u.string));
	strcpy(buf, (sp-1)->u.string);
	function_to_call_t *cb = MALLOC(sizeof(function_to_call_t));
	memset(cb, 0, sizeof(function_to_call_t));
	process_efun_callback(2, cb, F_ASYNC_WRITE);
	cb->f.fp->hdr.ref++;
	add_write(check_valid_path((sp-2)->u.string, current_object, "write_file", 1), buf, strlen(buf), 0, cb);
	pop_3_elems();
}
#endif
#endif
