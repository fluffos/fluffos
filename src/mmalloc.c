#include <sys/mman.h>
#include <unistd.h>

void *mmalloc(size_t size){
  register int *res = mmap(0,size+4,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,0,0);
  *res=size;
  return &res[1];
}

void mfree(void *p){
  register int *mem = p;
  mem--;
  munmap(mem, 4+*mem);
}

void *mrealloc(void *p, int size){
  register int *mem = p;
  int oldsize;
  mem--;
  oldsize = 4 + *mem;
  mem = mremap(mem, oldsize, size+4, 1);
  *mem = size;
  return &mem[1];
}
  
void *mcalloc(size_t num, size_t size){
  register void *p;

  size *= num;
  if ((p = mmalloc(size)))
    memset(p, 0, size);
  return (p);
}
