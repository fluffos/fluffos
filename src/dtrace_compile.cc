#include <stdlib.h>
#include "base/std.h"
int main() {
#ifdef DTRACE
#ifdef _LP64
  system("dtrace -G -64 -s fluffos.dtrace *.o packages/*.o > /dev/null");
#else
  system("dtrace -G -32 -s fluffos.dtrace *.o packages/*.o > /dev/null");
#endif
  printf("fluffos.o");
#endif
  return 0;
}
