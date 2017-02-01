package main

// #include <stdlib.h>  // for exit
// #include "main.h"
// #include "base/internal/external_port.h" // TODO
import "C"

import "fmt"
import "os"
import "flag"
import "syscall"
import "time"
import "unsafe"
import "net"
import "sync"
import "strconv"

func GDL_Lock() {
	GDL.Lock()
}

func GDL_Unlock() {
	GDL.Unlock()
}

var (
	fFlag  = flag.String("f", "", "master::flag to call")
	fDebug = flag.Int("d", 0, "Debug level")

	GDL   = &sync.Mutex{}
	conns = make(map[int]*net.TCPConn)
	connTotal int = 0
)

func main() {
	fmt.Println("=== FLUFFOS-GO Starting ===")
	fmt.Println("Boot Time: ", time.Now().String())
	fmt.Println("Full Command: ", os.Args)

	flag.Parse()

	{
		C.init_env()
		C.print_version_and_time()
	}

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

	{
		C.init_md()
	}
	fmt.Println("I'm here!")

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
		GDL_Lock()
		defer GDL_Unlock()

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

	cNewConn := make(chan NewConnEvent, 10)
	defer close(cNewConn)

	// Initialize user connection socket
	if ok, err := init_user_conn(cNewConn); !ok {
		fmt.Fprintf(os.Stderr, "Error Listening: %s", err)
		os.Exit(-1)
	}

	//if (has_console == 1) {
	//	console_init(base);
	//}

	fmt.Println("Initializations complete.")
	// TODO: setup_signal_handlers();

	C.real_main(base)

	d := time.Duration(int(C.get_gametick_ms())) * time.Millisecond
	tick := time.After(d)
	for {
		select {
		case <-tick:
			C.wrap_backend_once()
			tick = time.After(d)
		case c := <-cNewConn:
			handle_connection(c.idx, c.conn);
		}
	}
}

type NewConnEvent struct {
	idx int
	conn *net.TCPConn
}

func init_user_conn(c chan NewConnEvent) (bool, error) {
	for i := 0; i < (C.sizeof_external_port / C.sizeof_struct_port_def_t); i++ {
		// for F_NETWORK_STATS
		C.external_port[i].in_packets = 0
		C.external_port[i].in_volume = 0
		C.external_port[i].out_packets = 0
		C.external_port[i].out_volume = 0

		if C.external_port[i].port == 0 {
			continue
		}
		s := fmt.Sprintf("%s:%d", C.GoString(C.get_mud_ip()), C.external_port[i].port)
		laddr, err := net.ResolveTCPAddr("tcp", s)
		if err != nil {
			return false, err
		}
		ln, err := net.ListenTCP("tcp", laddr)
		if err != nil {
			return false, err
		}
		fmt.Printf("Accepting connection on %s.\n", ln.Addr().String())
		go func() {
			for {
				conn, err := ln.AcceptTCP()
				if err != nil {
					// do something
				}
				c <- NewConnEvent{i, conn}
			}
		}()
	}
	return true, nil
}

func handle_connection(idx int, conn *net.TCPConn) {
	// debug(connections, "New connection from %s.\n", sockaddr_to_string(addr, addrlen));
	connTotal = connTotal + 1
	connIdx := connTotal

	conns[connIdx] = conn

	// Setup some basics
	conn.SetKeepAlive(true)
	conn.SetKeepAlivePeriod(30 * time.Second)

	// set no delay
	conn.SetNoDelay(true)

	addr, port, err := net.SplitHostPort(conn.RemoteAddr().String())
	if err != nil {
		// not possible
	}

	{
		caddr := C.CString(addr)
		cport, err := strconv.Atoi(port)
		if err != nil {
			// not possible
		}
		// take ownership of caddr
		C.wrap_new_user_handler(C.int(idx), C.int(connIdx), caddr, C.int(cport))
	}

	// Call comm.cc:new_user_handler somehow
	// C.new_user_handler()

	//// TODO: merge event.cc into here.
	//new_user_event_listener(user);
	//// Initialize telnet support
	//user->telnet = net_telnet_init(user);
}

//export ConnWrite
func ConnWrite(idx int, data *C.char , len C.int) {
	b := C.GoBytes(unsafe.Pointer(data), len)
	conns[idx].Write(b)
}

//export ConnFlush
func ConnFlush(idx int) {
}

//export ConnClose
func ConnClose(idx int) {
	conns[idx].Close()
}