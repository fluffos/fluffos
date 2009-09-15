#include <stdlib.h>
#include "std.h"
int main(){
#ifdef DTRACE
#ifdef _LP64
  system("dtrace -G -64 -s fluffos.d obj/*.o > /dev/null");
#else
  system("dtrace -G -32 -s fluffos.d obj/*.o > /dev/null");
#endif
  printf("fluffos.o");
#endif
  return 0;
}
