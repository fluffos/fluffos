#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct freeblocks{
  unsigned long *block;
  struct freeblocks *next;
} *freelist = NULL;

static char *where = NULL;
static long blocksize = 0;

void *malloc64(int size){
  if(!blocksize){
	  char *tmp = (char *)sbrk(0); //end of heap
	  char *tmp2 = (char *)&size; //end of stack
	  char *tmp3 = (char *)0x4000000000; //libs, how do we get the actual address??
	  //printf("%ul %ul %ul", tmp, tmp2, tmp3);
	  if(tmp3 < tmp2 && tmp < tmp3){ //oops libraries in the middle, find the biggest gap we're assuming libs are smaller than 4GB total
	    if((tmp3 - tmp) > (tmp2 - tmp3)){
	      tmp2 = tmp3 - (long)4 * 1024 * 1024 * 1024;
	    } else {
	      tmp = tmp3 + (long)4 * 1024 * 1024 * 1024;
	    }
	  }
	    

	  long total = tmp2 - tmp; //memory available
	  total -= (long)4 * 1024 * 1024 * 1024; //leave some for the libc malloc (4GB)
	  blocksize = total/10000000; //10 million allocations of over 4k should be enough for anyone (at least 40 Gb requested at that point, in reality probably at least 10 times more!)
	  blocksize -= blocksize % 4096;
	  where = tmp + (long)4 * 1024 * 1024 * 1024;
	  where -= (long)where % 4096;
	  //printf("start %xl: blocksize: %xl\n", (long)where, blocksize);
  }

  if(size < 4088){
	  register unsigned long *res = (unsigned long *)malloc(size+sizeof(long));
	  if(!res){
		  perror("malloc: ");
		  exit(-1);
	  }
	  *res=size;
	  return &res[1];
  }

  register unsigned long *res;
  if(size < blocksize){
	  if(!freelist){
		  res = (unsigned long *)mmap(where, size+sizeof(long),MAP_FIXED|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		  if(res != where){
			  perror("mmap failed! (55)");
			  printf("where = %lx\n", where);
			  res = (unsigned long *)malloc(size+sizeof(long));
			  if(!res){
				  perror("malloc: ");
				  exit(-1);
			  }
			  *res=size;
			  return &res[1];
		  }
		  where += blocksize;
	  }else{
		  res = (unsigned long *)mmap(freelist->block, size+sizeof(long),MAP_FIXED|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
		  if(res != freelist->block)
			  perror("mmap failed! (60)");

		  struct freeblocks *tmp = freelist;
		  freelist = freelist->next;
		  free(tmp);
	  }
  } else {
	  //just in case something big comes along
	  int thissize = size / 4096;
	  thissize++;
	  res = (unsigned long *)mmap(where, size+sizeof(long),MAP_FIXED|PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
	  if(res != where)
		  perror("mmap failed! (72)");
	  where += (thissize*4096);
  }
  if((long)res == -1){
    perror("malloc: ");
    exit(-1);
  }
  *res=size;
  return &res[1];
}

void free64(void *p){
  register unsigned long *mem = (unsigned long *)p;
  mem--;
  if(mem < (unsigned long *)sbrk(0) || mem > (unsigned long *)where)
	  free(mem);
  else{
	  munmap(mem, *mem+sizeof(long));
	  struct freeblocks *bl = (struct freeblocks *)malloc(sizeof(struct freeblocks));
	  bl->block = mem;
	  bl->next = freelist;
	  freelist = bl;
  }
}

void *realloc64(void *p, int size){
  register unsigned long *mem = (unsigned long *)p;
  unsigned int oldsize;
  mem--;
  oldsize = *mem+sizeof(long);
  if(mem < (unsigned long *)sbrk(0) || mem > (unsigned long *)where){
	  if(size < 4088){
		  mem = (unsigned long *)realloc(mem, size + sizeof(long));
		  *mem = size;
		  return (void *)&mem[1];
	  } else {
		 mem++;
		 unsigned long *newmem = (unsigned long *)malloc64(size);
		 memcpy(newmem, mem, oldsize-sizeof(long));
		 mem--;
		 free(mem);
		 return (void *)newmem;
	  }
  }
  if(size < 4088){
	  mem++;
	  unsigned long *newmem = (unsigned long *)malloc(size+sizeof(long));
	  *newmem = size;
	  newmem++;
	  memcpy(newmem, mem, size);
	  free64(mem);
	  return (void *)newmem;
  }
  mem = (unsigned long *)mremap(mem, oldsize, size+sizeof(long), 1);
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
