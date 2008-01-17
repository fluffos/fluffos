#ifndef ASYNC_H_
#define ASYNC_H_
#include <aio.h>
#include "../function.h"

typedef struct aiocb aiob;

enum atypes {
		aread,
		awrite
};

struct request{
	aiob *aio;
	function_to_call_t *fun;
	struct request *next;
	enum atypes type;
};

void check_reqs();
#endif /*ASYNC_H_*/
