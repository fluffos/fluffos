package main

// #include <stdlib.h>  // for exit
// #include "main.h"
import "C"

import "fmt"
import "os"
import "flag"
import "syscall"
import "time"
import "unsafe"

var (
	fFlag  = flag.String("f", "", "master::flag to call")
	fDebug = flag.Int("d", 0, "Debug level")
)

func main() {
	fmt.Println("=== FLUFFOS-GO Starting ===")
	fmt.Println("Boot Time: ", time.Now().String())
	fmt.Println("Full Command: ", os.Args)

	flag.Parse()

	C.init_env()
	C.print_version_and_time()

	// show FD limits and core dump support.
	{
		r := syscall.Rlimit{Max: 65535}
		syscall.Setrlimit(syscall.RLIMIT_NOFILE, &r)
		syscall.Getrlimit(syscall.RLIMIT_NOFILE, &r)
		fmt.Printf("Max FD: %d\n", r.Cur)

		syscall.Getrlimit(syscall.RLIMIT_CORE, &r)
		c := "Yes"
		if r.Cur == 0 {
			c = "No"
		}
		fmt.Printf("Core Dump: %s\n", c)
	}

	fmt.Println("===========================")

	C.init_md()

	/* read in the configuration file */
	if flag.NArg() != 1 {
		fmt.Fprintf(os.Stderr, "Usage: %s config_file", os.Args[0])
		os.Exit(-1)
	}

	// Process configs
	{
		s := C.CString(flag.Arg(0))
		defer C.free(unsafe.Pointer(s))
		// FIXME: read_config may exit
		C.init_config(s)
	}

	// TODO: dump configs

	// Make sure mdlib dir is correct
	{
		s := C.GoString(C.get_mudlib_dir())
		if err := syscall.Chdir(s); err != nil {
			fmt.Fprintf(os.Stderr, "Bad mudlib directory: %s\n", s)
			os.Exit(-1)
		}
	}

	fmt.Println("Initializing internal stuff ....")
	base := C.init_libevent()
	C.init_vm()
	// process -d
	//if *fDebug {
	//	C.set_debug_level(*fDebug)
	//}
	// process -f
	if *fFlag != "" {
		s := C.CString(*fFlag)
		defer C.free(unsafe.Pointer(s))

		C.call_master_flag(s)
		if C.get_is_shutdown() != 0 {
			fmt.Println("Shutdown by master object.")
			os.Exit(0)
		}
	}

	// Initialize user connection socket
	//if (!init_user_conn()) {
	//	exit(1);
	//}

	//if (has_console == 1) {
	//	console_init(base);
	//}

	fmt.Println("Initializations complete.")
	// TODO: setup_signal_handlers();

	C.real_main(base)

}
