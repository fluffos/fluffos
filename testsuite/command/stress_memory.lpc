#define HOLDER "/clone/memory_holder"

void report() {
  write("After: \n" + mud_status(0) + "\n");
#ifdef __HAVE_JEMALLOC__
  dump_jemalloc();
  write("jemalloc dump done...\n");
#endif
}

int main(string arg) {
  write("Before: \n" + mud_status(0) + "\n");

  write("populate Map... \n\n");

  for(int i=0; i<100; i++) {
    HOLDER->populate_mixed(1);
    if(find_object(HOLDER)) {
      destruct(find_object(HOLDER));
    }
  }
  call_out("report", 1);
  reclaim_objects();

  return 1;
}
