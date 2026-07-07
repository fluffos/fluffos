varargs void two(string ref *values);
void one()
{
  two();  // Will this crash or cause a problem?
}

varargs void two(string ref *values)
{
  if(!values) {
  printf("Failure\n");
  return;
}
  values = allocate(1) ;
  values[0] = "boop" ;
}

void do_tests() {
  one();
}
