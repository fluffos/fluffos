#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct freeblocks{
  unsigned long *block;
  struct freeblocks *next;
} *freelist = NULL;

static char *where = NULL;
static long blocksize = 0;

void *malloc64(int size){
  if(!blocksize){
	  char *tmp = sbrk(0); //end of heap
	  char *tmp2 = (char *)&size; //end of stack
	  long total = tmp2 - tmp; //memory available
	  total -= (long)4 * 1024 * 1024 * 1024; //leave some for the libc malloc (4GB)
	  blocksize = total/10000000; //10 million allocations of over 4k should be enough for anyone (at least 40 Gb requested at that point, in reality probably at least 10 times more!)
	  blocksize -= blocksize % 4096;
	  where = tmp + (long)4 * 1024 * 1024 * 1024;
	  where -= (long)where % 4096;
	  printf("start %xl: blocksize: %xl\n", (long)where, blocksize);
  }

  if(size < 4088){
	  register unsigned long *res = malloc(size+sizeof(long));
	  if(!res){
		  perror("malloc: ");
		  exit(-1);
	  }
	  *res=size;
	  return &res[1];
  }

  register unsigned long *res;
  if(!freelist){
	  res = mmap(where, size+sizeof(long),MAP_FIXED|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
	  where += blocksize;
  }else{
	  res = mmap(freelist->block, size+sizeof(long),MAP_FIXED|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
	  struct freeblocks *tmp = freelist;
	  freelist = freelist->next;
	  free(tmp);
  }
  if((long)res == -1){
    perror("malloc: ");
    exit(-1);
  }
  *res=size;
  return &res[1];
}

void free64(void *p){
  register unsigned long *mem = p;
  mem--;
  if(mem < (unsigned long *)sbrk(0) || mem > (unsigned long *)where)
	  free(mem);
  else{
	  munmap(mem, *mem+sizeof(long));
	  struct freeblocks *bl = malloc(sizeof(struct freeblocks));
	  bl->block = mem;
	  bl->next = freelist;
	  freelist = bl;
  }
}

void *realloc64(void *p, int size){
  register unsigned long *mem = p;
  unsigned int oldsize;
  mem--;
  oldsize = *mem+sizeof(long);
  if(mem < (unsigned long *)sbrk(0) || mem > (unsigned long *)where){
	  if(size < 4088){
		  mem = realloc(mem, size + sizeof(long));
		  *mem = size;
		  return (void *)&mem[1];
	  } else {
		 mem++;
		 unsigned long *newmem = malloc64(size);
		 memcpy(newmem, mem, oldsize-sizeof(long));
		 mem--;
		 free(mem);
		 return (void *)newmem;
	  }
  }
  if(size < 4088){
	  mem++;
	  unsigned long *newmem = malloc(size+sizeof(long));
	  *newmem = size;
	  newmem++;
	  memcpy(newmem, mem, size);
	  free64(mem);
	  return (void *)newmem;
  }
  mem = mremap(mem, oldsize, size+sizeof(long), 1);
  *mem = size;
  return (void *)&mem[1];
}

void *calloc64(int num, int size){
  register void *p;
  size *= num;
  if ((p = malloc64(size)))
    memset(p, 0, size);
  return (p);
}
