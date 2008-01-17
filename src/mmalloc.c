#define _GNU_SOURCE
#include <sys/mman.h>
#include <unistd.h>

#define MINSIZE 0;//4*1024-8+rsize
//(long)5*1024*1024*1024-8+rsize

void *mmalloc(int rrsize){
  long rsize = rrsize;
  rsize = rsize % 4096 < 4088?(rsize/4096)*4096+4088:(rsize/4096)*4096+8180;
  long size = MINSIZE;
  size = size < rsize?rsize:size;
  register unsigned long *res = mmap(0,size+4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
  if(res == -1){
    perror("malloc: ");
    exit(-1);
  }
  mprotect(res+rsize, size-rsize, PROT_NONE);
  *res=size;
  return &res[1];
}

void mfree(void *p){
  register unsigned long *mem = p;
  mem--;
  munmap(mem, *mem+4);
  //mprotect(mem, 4+*mem, PROT_NONE);
}

void *mrealloc(void *p, int rrsize){
  long rsize = rrsize;
  register unsigned long *mem = p;
  unsigned int oldsize;
  rsize = rsize % 4096 < 4088?(rsize/4096)*4096+4088:(rsize/4096)*4096+8180;
  long size = MINSIZE;
  size = size < rsize?rsize:size;
  mem--;
  oldsize = *mem+8;
  mem = mremap(mem, oldsize, size+8, 1);
  mprotect(mem+rsize, size-rsize, PROT_NONE);
  *mem = size;
  return (void *)&mem[1];
}
  
void *mcalloc(int num, int size){
  register void *p;

  size *= num;
  if ((p = mmalloc(size)))
    memset(p, 0, size);
  return (p);
}
