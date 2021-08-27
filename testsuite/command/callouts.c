void future_function(int delay, int submitted)
{
  int now = time();
  printf("[%d] %ds delay call_out took %d seconds to land\n", now, delay, now - submitted) ;
}

int main()
{
  int num = -1 ;
  int now = time();
  printf("time: %d\n", now);
  while(num++ < 10)
    printf("%ds call_out, seconds remaining: %d\n",
           num, find_call_out(call_out("future_function", num, num, now))) ;

  printf("\n") ;
  return 1 ;
}
