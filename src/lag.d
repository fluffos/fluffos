BEGIN
{
  self->start = 0;
  self->ind = 0;
}

fluffos$target:::lpc-entry
/!self->ind/
{
  self->start = timestamp;
  self->spec = speculation();
  speculate(self->spec);
  printf("%s(%s):%s ", copyinstr(arg0), copyinstr(arg2), copyinstr(arg1));
}

fluffos$target:::lpc-entry
/self->ind==1/
{
  speculate(self->spec);
  printf("\n%s(%s):%s ", copyinstr(arg0), copyinstr(arg2), copyinstr(arg1));
}


fluffos$target:::lpc-entry
{
  self->ind++;
}

fluffos$target:::lpc-return
/self->ind>0/{
  self->ind--;
}

fluffos$target:::lpc-return
/!self->ind && timestamp - self->start>1000000000/
{
  speculate(self->spec);
  printf("%d\n", timestamp - self->start);
  commit(self->spec);
  self->spec = 0;
}

fluffos$target:::lpc-return
/!self->ind && self->spec/
{
  discard(self->spec);
  self->spec = 0;
}

