(char *)(((int) p + PAGESIZE-1) & ~(PAGESIZE-1));

int *ad;
int *al;
int size;

void *alignedmalloc(int size){
  void *tmp;
  ad[size] = tmp = malloc(size+PAGESIZE-1);
  al[size++] = tmp = (void *)(((int) tmp + PAGESIZE-1) & ~(PAGESIZE-1));
  return tmp;
}


