#include <pthread.h>
static phread_t async_thread;

void *async_loop PROT((void));

void init_async PROT((void)){
  phtread_create(&async_thread, 0, async_loop, 0);
}

void *async_loop PROT((void)){
  