package main

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"net"
	"os"
	"strconv"
	"sync"
	"syscall"
	"time"
	"unsafe"
)

// #include <stdlib.h>  // for exit
// #include "main.h"
// #include "base/internal/external_port.h" // TODO
// #include "thirdparty/libtelnet/libtelnet.h"
import "C"

var (
	fFlag  = flag.String("f", "", "master::flag to call")
	fDebug = flag.Int("d", 0, "Debug level")

	users         = make(map[int]*User)
	userTotal int = 0
	userMutex     = &sync.Mutex{} // protect users and userTotal

	// Channel for run a callback in main driver loop, thus avoiding the GDL.
	cBlockingRunInMainLoop = make(chan func(), 1)

	//cUserDisconnect = make(chan *UserDisconnectEvent, 10)
)

func BlockingRunInMainLoop(f func()) {
	done := make(chan bool)
	cBlockingRunInMainLoop <- func() {
		f()
		done <- true
	}
	<-done
}

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
		r := syscall.Rlimit{Cur: 65535, Max: 65535}
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
		// FIXME: read_config may exit
		C.init_config(s)
		C.free(unsafe.Pointer(s))
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

	// Channel for watching User input
	defer close(cBlockingRunInMainLoop)

	// this is the main loop, everything that calls into VM must be processed here,
	// this queue serves as an global lock.
	for {
		select {
		case <-tick:
			C.wrap_backend_once()
			tick = time.After(d)
		case callback := <-cBlockingRunInMainLoop:
			callback() // executing main loop callback
		}
	}
}

type NewConnEvent struct {
	idx  int
	conn *net.TCPConn
}

type NewUserInputEvent struct {
	ip    *C.struct_interactive_t
	input []byte
}

//
//Here is the plan:
//
//1. netTCPConn receive -> run telnet_recv() in main loop.
//
//	3. Implement an buffer for on_telnet_data to write in,
// in main loop.  userInput need to return bytes consumed. (maybe 0 or 1)
//
//	4. instead of use telnet_send,  provide a telnet_send_async()
// that goroutine that writes to a PipeWriter and wakeup a io.Copy()
// goroutine to call telnet_send().
// 5. on_telnet_send just call netTCPConn.Write()

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
		go func(idx int, ln *net.TCPListener) {
			for {
				conn, err := ln.AcceptTCP()
				if err != nil {
					// do something
				}
				// process new connection
				go handle_connection(idx, conn)
			}
		}(i, ln)
	}
	return true, nil
}

type User struct {
	idx int

	// The processed telnet text data buffer
	buf *bytes.Buffer

	ip     *C.struct_interactive_t
	telnet *C.struct_telnet_t

	netTCPConn *net.TCPConn
}

func handle_connection(port_idx int, conn *net.TCPConn) {
	// debug(connections, "New connection from %s.\n", sockaddr_to_string(addr, addrlen));
	fmt.Printf("New Connection %d from: %s.\n", port_idx, conn.RemoteAddr().String())

	// Setup some basics
	conn.SetKeepAlive(true)
	conn.SetKeepAlivePeriod(30 * time.Second)
	conn.SetNoDelay(true)

	// This need to be done before calling new_user_handler.
	var user *User

	{
		userMutex.Lock()

		userIdx := userTotal
		userTotal = userTotal + 1
		user = &User{
			idx:        userIdx,
			buf:        &bytes.Buffer{},
			ip:         nil,
			telnet:     nil,
			netTCPConn: conn,
		}
		users[userIdx] = user

		userMutex.Unlock()
	}

	// Initilizaitions
	{
		addr, port, err := net.SplitHostPort(conn.RemoteAddr().String())
		if err != nil {
			// not possible
		}
		caddr := C.CString(addr)
		cport, err := strconv.Atoi(port)
		if err != nil {
			// not possible
		}
		// take ownership of caddr
		// block
		res := &C.struct_new_user_result_t{}
		BlockingRunInMainLoop(func() {
			*res = C.wrap_new_user_handler(C.int(port_idx), C.int(user.idx), caddr, C.int(cport))
		})
		if res.user == nil {
			// Didn't return a IP pointer meaning the connection was terminated.
			conn.Close()
			userMutex.Lock()
			delete(users, user.idx)
			userMutex.Unlock()
			return
			// deal with error cases
		}
		user.ip = res.user
		user.telnet = res.telnet
	}

	// creating context

	// Buffer for telnet_recv
	buf := make([]byte, 4096)

	// How many bytes we already peeked in the last round, we only
	// process new bytes that arrives.
	lastPeeked := 0

	// now we just need to wait for Text data to appear in our buffer!
	for {
		// Reading from TCP connection
		{
			n, err := user.netTCPConn.Read(buf)
			if n <= 0 && err != nil {
				// Connection gone bad.
				break
			}
			fmt.Printf("calling telnet_recv with %v (len: %d)\n", buf[:n], n)
			BlockingRunInMainLoop(func() {
				C.telnet_recv(user.telnet, (*C.char)(unsafe.Pointer(&(buf[:n][0]))), C.size_t(n))
				// will call onTelnetData
			})
			fmt.Printf("telnet_recv done.\n")
		}

		fmt.Printf("Starting to wait for command! \n")

		if user.buf.Len() <= lastPeeked {
			fmt.Println("No more data in buffer, waiting")
			continue
		}
		fmt.Printf("We've got data, buffer content: %v \n", user.buf.Bytes())

		// skip null byte
		for {
			b, err := user.buf.ReadByte()
			if err == io.EOF {
				// reached end
				goto finish
			}
			if b == byte(0) {
				continue
			} else {
				user.buf.UnreadByte()
				break
			}
		}

		// Unfortunelty, the real processing need to be done in main loop for now.
		// due to accessing various flags.
		BlockingRunInMainLoop(func() {
			/* if we're in single character mode, we've got input */
			var command []byte

			if C.is_single_char(user.ip) != 0 {
				// Skip ASCII code 8 and 127 (I don't know why, but the
				// existing code does this.
				for {
					b, err := user.buf.ReadByte()
					if err == io.EOF {
						// reached end, no valid command
						return
					}
					if b == byte(0) || b == byte(8) || b == byte(127) {
						continue
					} else {
						command = []byte{b}
						break
					}
				}
			} else {
				// This is just for peeking
				raw := user.buf.Bytes()

				i := lastPeeked // start from where last time ends
				for ; i < len(raw); i++ {
					if raw[i] == '\r' || raw[i] == '\n' {
						break
					}
				}
				if i == len(raw) { // not found
					// No command this time, wait for next read
					lastPeeked = i
					return
				}

				// check next character , if it is part of '\n\r' or
				// '\r\n', then consume that too
				if i+1 < len(raw) &&
					((raw[i] == '\r' && raw[i+1] == '\n') || (raw[i] == '\n' && raw[i+1] == '\r')) {
					i = i + 1
				}

				// Now that we formally have a command, formally read it out from buffer.
				raw = user.buf.Next(i + 1)
				lastPeeked = 0

				// trim the end on first '\r','\n' or NUL byte.
				for i = 0; i < len(raw); i++ {
					if raw[i] == byte(0) || raw[i] == '\r' || raw[i] == '\n' {
						break
					}
				}
				raw = raw[:i]

				// Construct real user command
				ANSI_SUBSTITUTE := byte(0x20)
				for i := 0; i < len(raw); i++ {
					c := raw[i]
					switch c {
					case 0x1b:
						if C.is_no_ansi_and_strip() != 0 {
							command = append(command, ANSI_SUBSTITUTE)
							break
						}
					default:
						command = append(command, c)
						break
					}
				}
			}
			// add trialing NULL
			command = append(command, 0)
			fmt.Printf("Got command: %v.\n", command)
			// we can simply just execute it here
			tmp := C.CBytes(command)
			C.wrap_on_user_command(user.ip, (*C.char)(tmp))
			C.free(tmp)
		})
		// we processed everything, move on waiting for more data
	finish:
	}
	// if we reach here, meaning connection dead.
	// clean up!
	user.netTCPConn.Close()
	// Deal with errors.
	// Notify VM
	BlockingRunInMainLoop(func() {
		C.on_conn_error(user.ip)
	})
	userMutex.Lock()
	delete(users, user.idx)
	userMutex.Unlock()
	user.ip = nil
	user.telnet = nil
	// Done
}

//export OnTelnetData
func OnTelnetData(idx C.int, buffer *C.char, size C.int) {
	userMutex.Lock()
	user, ok := users[int(idx)]
	userMutex.Unlock()

	if !ok {
		return
	}

	// the buffer here comes from Write() function above, use this trick to avoid
	// Copy, since we just need to write it to buffer anyway.
	buf := (*[1 << 30]byte)(unsafe.Pointer(buffer))[:size:size]
	user.buf.Write(buf)
}

//export ConnWrite
func ConnWrite(idx C.int, data *C.char, l C.int) {
	userMutex.Lock()
	user, ok := users[int(idx)]
	userMutex.Unlock()

	if !ok {
		return
	}

	buf := (*[1 << 30]byte)(unsafe.Pointer(data))[:l:l]
	fmt.Printf("ConnWrite: %v, (len: %d) \n", buf, len(buf))
	user.netTCPConn.Write(buf)
}

//export ConnFlush
func ConnFlush(idx C.int) {
}

//export ConnClose
func ConnClose(idx C.int) {
	userMutex.Lock()
	user, ok := users[int(idx)]
	userMutex.Unlock()

	if !ok {
		return
	}

	user.netTCPConn.Close()
}
