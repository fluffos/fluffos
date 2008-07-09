#include "std.h"
#include "socket_ctrl.h"
#include "network_incl.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "file.h"

/*
  ioctl.c: part of the MudOS release -- Truilkan@TMI

  isolates the code which sets the various socket modes since various
  machines seem to need this done in different ways.
*/

/*
 * set process receiving SIGIO/SIGURG signals to us.
 */

INLINE int set_socket_owner (int fd, int which)
{
#if defined(OLD_ULTRIX) || defined(__CYGWIN__)
    return fcntl(fd, F_SETOWN, which);
#else
#ifdef WINSOCK
    return 1; /* FIXME */
#else
    return ioctl(fd, SIOCSPGRP, &which);
#endif
#endif
}

/*
 * allow receipt of asynchronous I/O signals.
 */

INLINE int set_socket_async (int fd, int which)
{
#ifdef OLD_ULTRIX
    return fcntl(fd, F_SETFL, FASYNC);
#else
    return OS_socket_ioctl(fd, FIOASYNC, &which);
#endif
}

/*
 * set socket non-blocking
 */

INLINE int set_socket_nonblocking (int fd, int which)
{
#if !defined(OLD_ULTRIX) && !defined(_SEQUENT_)
    int result;
#endif

#ifdef OLD_ULTRIX
    if (which)
	return fcntl(fd, F_SETFL, FNDELAY);
    else
	return fcntl(fd, F_SETFL, FNBLOCK);
#else

#ifdef _SEQUENT_
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags == -1)
	return (-1);
    if (which)
	flags |= O_NONBLOCK;
    else
	flags &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
#else
    result = OS_socket_ioctl(fd, FIONBIO, &which);
    if (result == -1)
	debug_perror("set_socket_nonblocking: ioctl", 0);
#if 0
    /* boggle ... someone track down an errno for this */
    if (result == -1) {
	XXX("Try using cc instead of gcc to correct this error.\n");
    }
#endif
    return result;
#endif

#endif
}

#ifdef WIN32
void SocketPerror (char * what, char * file) {
    static char *errstrings[] =
    {  "Operation would block",
       "Blocking call in progress",
       "WSAEALREADY",
       "Invalid socket",
       "Missing destination",
       "Data is too large",
       "Wrong protocol type",
       "Unsupported option",
       "Unsupported protocol",
       "Unsupported socket type",
       "Socket can't listen",
       "WSAEPFNOSUPPORT",
       "Can't use address family",
       "Addr is used",
       "Addr is not available",
       "WSAENETDOWN",
       "WSAENETUNREACH",
       "WSAENETRESET",
       "WSAECONNABORTED",
       "WSAECONNRESET",
       "No buffer space",
       "Already connected",
       "Not connected",
       "WSAESHUTDOWN",
       "WSAETOOMANYREFS",
       "Time-out",
       "Connection refused",
       "WSAELOOP",
       "WSAENAMETOOLONG",
       "WSAEHOSTDOWN",
       "WSAEHOSTUNREACH",
       "10066", "10067", "10068", "10069", "10070", "10071", "10072", "10073", "10074",
       "10075", "10076", "10077", "10078", "10079", "10080", "10081", "10082", "10083",
       "10084", "10085", "10086", "10087", "10088", "10089", "10090",
       "WSASYSNOTREADY",
       "WSAVERNOTSUPPORTED",
       "Winsock not initialised",
       "10094", "10095", "10096", "10097", "10098", "10099", "11000",
       "WSAHOST_NOT_FOUND",
       "WSATRY_AGAIN",
       "WSANO_RECOVERY",
       "WSANO_DATA"
    };

    static char tmpstring[80];
    char *s = tmpstring;
    int sock_errno;

    sock_errno = WSAGetLastError();

    switch (sock_errno) {
        case WSAEINTR:  strcpy(tmpstring, "Function interrupted");
        case WSAEACCES: strcpy(tmpstring, "Cannot broadcast");
        case WSAEFAULT: strcpy(tmpstring, "Buffer is invalid");
        case WSAEINVAL: strcpy(tmpstring, "Unbound socket");
        case WSAEMFILE: strcpy(tmpstring, "No more descriptors");

        default:
            if ( (sock_errno >= WSAEWOULDBLOCK) && (sock_errno <= WSANO_DATA)) {
                s = errstrings[sock_errno - WSAEWOULDBLOCK];
		} else
                strcpy(tmpstring, "unknown error");
    }
    if (file)
        debug_message("System Error: %s:%s:%s\n", what, file, s);
    else
        debug_message("System Error: %s:%s\n", what, s);
}

void CDECL cleanup_sockets(void) {
	WSACleanup();
}
#endif
