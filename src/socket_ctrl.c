#include "std.h"
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

INLINE int set_socket_owner P2(int, fd, int, which)
{
#ifdef OLD_ULTRIX
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

INLINE int set_socket_async P2(int, fd, int, which)
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

INLINE int set_socket_nonblocking P2(int, fd, int, which)
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
