/*
  ioctl.c: part of the MudOS release -- Truilkan@TMI

  isolates the code which sets the various socket modes since various
  machines seem to need this done in different ways.
*/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#ifdef _SEQUENT_
#include <fcntl.h>
#endif
#if defined(SVR4)
#include <fcntl.h>
#include <sys/filio.h>
#include <sys/sockio.h>
#endif
#include "config.h"
#include "lint.h"

/*
 * set process receiving SIGIO/SIGURG signals to us.
 */

INLINE int set_socket_owner(fd, which)
int fd, which;
{
	return ioctl(fd, SIOCSPGRP, &which);
}

/*
 * allow receipt of asynchronous I/O signals.
 */

INLINE int set_socket_async(fd, which)
int fd, which;
{
	return ioctl(fd, FIOASYNC, &which);
}

/*
 * set socket non-blocking
 */

INLINE int set_socket_nonblocking(fd, which)
int fd, which;
{
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
	return ioctl(fd, FIONBIO, &which);
#endif
}
