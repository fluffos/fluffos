#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

void *malloc32(int size){
  register unsigned long *res = (unsigned long *)malloc(size+sizeof(long));
  if(!res){
    perror("malloc: ");
    exit(-1);
  }
  *res=size;
  return &res[1];
}

void free32(void *p){
  register unsigned long *mem = (unsigned long *)p;
  mem--;
  free(mem);
}

void *realloc32(void *p, int size){
  register unsigned long *mem = (unsigned long *)p;
  unsigned int oldsize;
  oldsize = mem[-1];
  void *newmem = malloc32(size);
  oldsize = oldsize<size?oldsize:size;
  memcpy(newmem, mem, oldsize);
  free(--mem);
  return (void *)newmem;
}

void *calloc32(int num, int size){
  register void *p;
  size *= num;
  if ((p = malloc32(size)))
    memset(p, 0, size);
  return (p);
}
