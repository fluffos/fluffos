package main

//#include <stdlib.h>
//#include <stdio.h>
//static void* allocArgv(int argc) {
//  return malloc(sizeof(char *) * argc);
//}
//#include "main.h"
import "C"
import "fmt"
import "unsafe"
import "os"

func main() {
	fmt.Println("=== FLUFFOS-GO 0.0.1 ===")
	argv := os.Args
	argc := C.int(len(argv))
	c_argv := (*[0xfff]*C.char)(C.allocArgv(argc))
	defer C.free(unsafe.Pointer(c_argv))
	for i, arg := range argv {
		c_argv[i] = C.CString(arg)
		defer C.free(unsafe.Pointer(c_argv[i]))
	}
	fmt.Println(C.real_main(argc, unsafe.Pointer(c_argv)))
}
