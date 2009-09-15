#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void *malloc32(int size){
  register unsigned long *res = malloc(size+sizeof(long));
  if(!res){
    perror("malloc: ");
    exit(-1);
  }
  *res=size;
  return &res[1];
}

void free32(void *p){
  register unsigned long *mem = p;
  mem--;
  free(mem);
}

void *realloc32(void *p, int size){
  register unsigned long *mem = p;
  unsigned int oldsize;
  mem--;
  oldsize = *mem+sizeof(long);
  mem++;
  unsigned long *newmem = malloc32(size);
  size += sizeof(long);
  oldsize = oldsize<size?oldsize:size;
  memcpy(newmem, mem, oldsize-sizeof(long));
  mem--;
  free(mem);
  return (void *)newmem;
}

void *calloc32(int num, int size){
  register void *p;
  size *= num;
  if ((p = malloc32(size)))
    memset(p, 0, size);
  return (p);
}
