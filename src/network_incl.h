#ifndef NETWORK_INCL_H
#define NETWORK_INCL_H

#if !defined(LATTICE) && !defined(OS2)
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  ifndef _AUX_SOURCE
#    include <arpa/telnet.h>
#  endif
#  ifdef _CX_UX
#    include <sys/sema.h>
#  endif
#  if !defined(appolo) && !defined(linux) && !defined(_M_UNIX)
#    include <sys/socketvar.h>
#  endif
#endif

#ifdef LATTICE
#  include <socket.h>
#endif

#if defined(_AUX_SOURCE) || defined(__SASC)
#  include "telnet.h"
#endif

#ifdef _AUX_SOURCE
#  include <resolv.h>
#endif

#endif
