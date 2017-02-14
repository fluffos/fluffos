package main

import (
	"bytes"
	"flag"
	"fmt"
	"io"
	"net"
	"os"
	"os/signal"
	"sync"
	"syscall"
	"time"
	"unsafe"
	"encoding/binary"
)

// #include <stdlib.h>  // for exit
// #include "main.h"
// #include "base/internal/external_port.h" // TODO
// #include "thirdparty/libtelnet/libtelnet.h"
import "C"

var (
	fFlag  = flag.String("f", "", "master::flag to call")
	fDebug = flag.Int("d", 0, "Debug level")

	users         = make(map[int]*TelnetUser)
	userTotal int = 0
	userMutex     = &sync.Mutex{} // protect users and userTotal

	// Channel for run a callback in main driver loop, thus avoiding the GDL.
	cBlockingRunInMainLoop = make(chan func())
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
	flag.Parse()

	if flag.NArg() != 1 {
		fmt.Fprintf(os.Stderr, "Usage: %s config_file", os.Args[0])
		os.Exit(-1)
	}

	C.init_env()

	fmt.Println("=== FLUFFOS-GO Starting ===")
	fmt.Println("Boot Time: ", time.Now().String())
	fmt.Println("Full Command: ", os.Args)
	C.print_version_and_time()

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

	C.init_md()

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
	}

	// setup signal handler, on these signal will try to exit driver gracefully
	cSigShutdown := make(chan os.Signal, 1)
	signal.Notify(cSigShutdown, os.Interrupt, syscall.SIGHUP)

	cSigAbort := make(chan os.Signal, 1)
	signal.Notify(cSigAbort, syscall.SIGQUIT, syscall.SIGABRT, syscall.SIGILL, syscall.SIGSYS)

	CSigUser := make(chan os.Signal, 1)
	signal.Notify(CSigUser, syscall.SIGUSR1, syscall.SIGUSR2)

	// Register attempt_shutdown for all other cases that will run during panic
	defer func() {
		tmp := C.CString("FluffOS aborted.")
		defer C.free(unsafe.Pointer(tmp))
		C.wrap_call_fatal(tmp)
	}()

	// Initialize user connection socket
	if ok, err := init_user_conn(); !ok {
		fmt.Fprintf(os.Stderr, "Error Listening: %s", err)
		os.Exit(-1)
	}

	//if (has_console == 1) {
	//	console_init(base);
	//}

	fmt.Println("Initializations complete.")
	// TODO: setup_signal_handlers();

	C.init_backend(base)

	d := time.Duration(int(C.get_gametick_ms())) * time.Millisecond
	tick := time.After(d)

	// Channel for watching User input
	defer close(cBlockingRunInMainLoop)

	// this is the main loop, everything that calls into VM must be processed here,
	// this queue serves as an global lock.
	for {
		select {
		case <-cSigShutdown:
			fmt.Println("Process Interrupted, shutting down.")
			goto shutdown
		case c := <-cSigAbort:
			signal.Stop(cSigAbort)
			fmt.Fprintf(os.Stderr, "Shutdown caused by Signal %s.\n", c.String())
			// run deferred abort sequence.
			return
		case c := <-CSigUser:
			if c == syscall.SIGUSR1 {
				C.wrap_call_crash()
				os.Exit(-1) // will not run defer
			}
			if c == syscall.SIGUSR2 {
				C.wrap_set_outoftime(1)
			}
		case <-tick:
			C.wrap_backend_once()
			tick = time.After(d)
		case callback := <-cBlockingRunInMainLoop:
			callback() // executing main loop callback
		}
	}

	shutdown:
	// Shutdown
	C.wrap_shutdownMudOS(0)
	os.Exit(0) // will not run defer
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

func init_user_conn() (bool, error) {
	for i := 0; i < (C.sizeof_external_port / C.sizeof_struct_port_def_t); i++ {
		port := C.external_port[i]

		// skip unconfigured port
		if port.port == 0 {
			continue
		}

		s := fmt.Sprintf("%s:%d", C.GoString(C.get_mud_ip()), port.port)
		laddr, err := net.ResolveTCPAddr("tcp", s)
		if err != nil {
			return false, err
		}
		ln, err := net.ListenTCP("tcp", laddr)
		if err != nil {
			return false, err
		}
		fmt.Printf("Accepting connection on %s.\n", ln.Addr().String())
		go func(port_idx int, port_type int, ln *net.TCPListener) {
			for {
				conn, err := ln.AcceptTCP()
				if err != nil {
					// do something
					continue
				}
				fmt.Printf("New Connection on port %d from: %s.\n", port_idx, conn.RemoteAddr().String())

				// Setup keepalive for all connections.
				conn.SetKeepAlive(true)
				conn.SetKeepAlivePeriod(30 * time.Second)

				if (port_type == PORT_TELNET) {
					conn.SetNoDelay(true)
				}

				go handle_connection(port_idx, port_type, conn)
			}
		}(i, int(port.kind), ln)
	}
	return true, nil
}

type TelnetUser struct {
	idx int

	// The processed telnet text data buffer, it is only ever
	// accessed by one thread, either through telnet_recv
	// (OnTelnetData) or in the connection goroutine.
	buf *bytes.Buffer

	ip     *C.struct_interactive_t
	telnet *C.struct_telnet_t

	netTCPConn *net.TCPConn
}

func NewUser(port_idx int, conn *net.TCPConn) *TelnetUser {
	userMutex.Lock()

	userIdx := userTotal
	userTotal = userTotal + 1
	user := &TelnetUser{
		idx:        userIdx,
		buf:        &bytes.Buffer{},
		ip:         nil,
		telnet:     nil,
		netTCPConn: conn,
	}
	users[userIdx] = user

	userMutex.Unlock()

	// Initializations
	{
		hostport := C.CString(conn.RemoteAddr().String())
		res := &C.struct_new_user_result_t{}
		BlockingRunInMainLoop(func() {
			// take ownership of hostport
			*res = C.wrap_new_user_handler(C.int(port_idx), C.int(user.idx), hostport)
		})
		if res.user == nil {
			// Didn't return a IP pointer meaning the connection was terminated.
			return nil
		}
		user.ip = res.user
		user.telnet = res.telnet
	}

	return user;
}

func DeleteUser(user *TelnetUser) {
	user.netTCPConn.Close()
	userMutex.Lock()
	delete(users, user.idx)
	userMutex.Unlock()
}

func handle_connection(port_idx int, port_type int, conn *net.TCPConn) {
	user := NewUser(port_idx, conn)
	if user == nil {
		return
	}
	defer DeleteUser(user)

	buf := make([]byte, 4096)
	for {
		n, err := user.netTCPConn.Read(buf)
		if n <= 0 && err != nil {
			BlockingRunInMainLoop(func() {
				C.on_conn_error(user.ip)
			})
			return
		}

		// Network accounting
		RecordIngress(port_idx, uint64(n))

		switch(port_type) {
		case PORT_TELNET:
			fmt.Printf("calling telnet_recv with %v (len: %d)\n", buf[:n], n)
			BlockingRunInMainLoop(func() {
				C.telnet_recv(user.telnet, (*C.char)(unsafe.Pointer(&(buf[:n][0]))), C.size_t(n))
				// will call onTelnetData
			})
			fmt.Printf("telnet_recv done.\n")
			handle_telnet_read(user);
		case PORT_BINARY:
			user.buf.Write(buf)
			handle_binary_read(user)
		case PORT_ASCII:
			user.buf.Write(buf)
			handle_ascii_read(user)
		case PORT_MUD:
			user.buf.Write(buf)
			handle_mud_read(user)
		case PORT_WEBSOCKET:
		// do something
		}
	}
}

func handle_mud_read(user *TelnetUser) {
	// Mud protocol is that every message is 4 bytes length + data.
	shouldContinue := true
	for shouldContinue {
		buf := user.buf.Bytes()
		size :=  binary.BigEndian.Uint32(buf[:4])
		// TODO: for security, here should have some limit on size
		if len(buf) < int(uint32(4) + size) {
			shouldContinue = false
			return
		}
		// Process it
		_ = user.buf.Next(4)
		buf = user.buf.Next(int(size))
		buf = append(buf, byte(0))

		BlockingRunInMainLoop(func() {
			C.wrap_on_mud_data(user.ip, (*C.char)(unsafe.Pointer(&buf[0])))
		})

		shouldContinue = user.buf.Len() > 0
	}
}

func handle_ascii_read(user *TelnetUser) {
	shouldContinue := true
	for shouldContinue {
		buf := user.buf.Bytes()
		// Look for '\n' in the buffer
		if i := bytes.IndexByte(buf, '\n') ; i != -1 {
			buf = user.buf.Next(i+1)
			// remove last '\n'
			buf = buf[:len(buf)-1]
			// If there is a '\r', get rid of it too.
			if buf[len(buf)-1] == '\r' {
				buf = buf[:len(buf) - 1 ]
			}
			buf = append(buf, byte(0))
			BlockingRunInMainLoop(func() {
				C.wrap_on_ascii_data(user.ip, (*C.char)(unsafe.Pointer(&buf[0])))
			})
			shouldContinue = user.buf.Len() > 0
		} else {
			shouldContinue = false
			return
		}
	}
}

func handle_binary_read(user *TelnetUser) {
	buf := user.buf.Next(user.buf.Len())

	BlockingRunInMainLoop(func() {
		C.wrap_on_binary_data(user.ip, (*C.char)(unsafe.Pointer(&buf[0])), C.int(len(buf)))
	})
}

func handle_telnet_read(user *TelnetUser) {
	shouldContinue := true

	for shouldContinue {
		// skip null byte
		for {
			b, err := user.buf.ReadByte()
			if err == io.EOF {
				shouldContinue = false;
				return
			}
			if b == byte(0) {
				continue
			} else {
				user.buf.UnreadByte()
				break
			}
		}

		// Unfortunately, the real processing need to be done in main loop for now.
		// due to accessing various flags.
		BlockingRunInMainLoop(func() {
			var command []byte

			/* if we're in single character mode, we've got input */
			if C.is_single_char(user.ip) != 0 {
				// Skip ASCII code 8 and 127 (I don't know why, but the
				// existing code does this.
				for {
					b, err := user.buf.ReadByte()
					if err == io.EOF {
						// reached end, no valid command
						shouldContinue = false
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

				i := 0 // start from where last time ends
				for ; i < len(raw); i++ {
					if raw[i] == '\r' || raw[i] == '\n' {
						break
					}
				}
				if i == len(raw) {
					// not found
					// No command this time, wait for next read
					shouldContinue = false
					return
				}
				// check next character , if it is part of '\n\r' or
				// '\r\n', then consume that too
				if i + 1 < len(raw) &&
					((raw[i] == '\r' && raw[i + 1] == '\n') || (raw[i] == '\n' && raw[i + 1] == '\r')) {
					i = i + 1
				}
				// Now that we formally have a command, formally read it out from buffer.
				raw = user.buf.Next(i + 1)

				// trim the end on first '\r','\n' or NUL byte.
				for i = 0; i < len(raw); i++ {
					if raw[i] == byte(0) || raw[i] == '\r' || raw[i] == '\n' {
						break
					}
				}
				command = raw[:i]
			}
			// add trialing NULL
			command = append(command, 0)
			fmt.Printf("Got command: %v.\n", command)
			// we can simply just execute it here
			tmp := C.CBytes(command)
			C.wrap_on_user_command(user.ip, (*C.char)(tmp))
			C.free(tmp)
		})

		shouldContinue = user.buf.Len() > 0
	}
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


	// Do ANSI SUBSTITUTION
	if C.is_no_ansi_and_strip() != 0 {
		ANSI_SUBSTITUTE := byte(0x20)
		for i := 0; i < len(buf); i++ {
			c := buf[i]
			switch c {
			case 0x1b:
				buf[i] = ANSI_SUBSTITUTE
				break
			default:
			}
		}
	}

	user.buf.Write(buf)
}

//export ConnWrite
func ConnWrite(idx C.int, port_idx C.int, data *C.char, l C.int) {
	userMutex.Lock()
	user, ok := users[int(idx)]
	userMutex.Unlock()

	if !ok {
		return
	}

	buf := (*[1 << 30]byte)(unsafe.Pointer(data))[:l:l]
	fmt.Printf("ConnWrite: %v, (len: %d) \n", buf, len(buf))
	user.netTCPConn.Write(buf)

	// TODO: pass port here for correct network accounting
	RecordEgress(int(port_idx), uint64(l))
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
