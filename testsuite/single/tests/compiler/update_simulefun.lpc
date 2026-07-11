#define SIMULEFUN "/single/simulefun"
void update_simulefun() {
  object obj;
  if(obj = find_object(SIMULEFUN)) {
    destruct(obj);
  }
  load_object(SIMULEFUN);
}

void do_tests() {
  ASSERT_EQ(0, catch(update_simulefun()));
}


