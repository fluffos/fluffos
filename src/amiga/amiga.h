/* Amiga include file for mudos 
 * By Wildcard (umfehr06@ccu.umanitoba.ca
 */

#ifndef AMIGA_H
#define AMIGA_H
#define SIGHUP  29
#define SIGPIPE 13 
#define _IFIFO  0010000 
#define _IFCHR  0020000 
#define _IFBLK  0060000 
#define _IFLNK  0120000 
#define S_IFCHR _IFCHR
#define S_IFBLK _IFBLK
#define S_IFIFO _IFIFO
#define S_IFLNK _IFLNK

#define NBBY    8               
#define FD_SETSIZE      256
typedef long    fd_mask;
#define NFDBITS (sizeof (fd_mask) * NBBY)       
#define howmany(x, y)   (((x)+((y)-1))/(y))
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      bzero((char *)(p), sizeof (*(p)))

typedef struct fd_set {
        fd_mask fds_bits[howmany(FD_SETSIZE, NFDBITS)];
} fd_set;

#define TELOPT_ECHO 1                 
#define TELOPT_SGA  3   
#define TELOPT_TM   6

#define SE      240
#define DM  	242     
#define BREAK   243
#define IP      244
#define AO      245
#define AYT     246
#define SB      250
#define WILL    251     
#define WONT    252     
#define DO  	253     
#define DONT    254     
#define IAC 	255     
#define MSG_OOB     0x1
#define MAXPATHLEN   1024
#define EWOULDBLOCK  35               
#define EINPROGRESS  36              
#define EALREADY     37
#define ECONNREFUSED 61              
#define _IOC_IN         0x80000000      
#define _IOCPARM_MASK   0xff            
#define _IOW(x,y,t)     (_IOC_IN|((sizeof(t)&_IOCPARM_MASK)<<16)|('x'<<8)|y)

#define SIOCSPGRP    _IOW(s,  8, int)            

/* stuff that's ignored */
#define gethostbyaddr(a,l,t) (0)
#define getsockname(s,n,l) (0)
#define ioctl(socket,code,p) (0)
#define listen(socket,queue_size) (0)
#define connect(s,a,n) (-1)

#define htonl(x)        (x)
#define ntohs(x)        (x)
#define major(x)        ((int)(((unsigned)(x)>>8)&0377))
#define minor(x)        ((int)((x)&0377))
#define recv(s,b,l,f) read_socket(s,b,l)
#define recvfrom(s,b,c,f,a,l) read_socket(s,b,c)
#define send(s,m,l,f) write_socket(s,m,l)                              
#define sendto(s,m,n,f,a,l) write_socket(s,m,n)                              

/* some varargs stuff (normal stdargs.h doesn't work) */
#define va_dcl int va_alist;
#define va_start(list) list = (char *) &va_alist
#define va_arg(a,b)  (*((b *) ((a +=  ((sizeof(b)+sizeof(int)-1) & ~(sizeof(int)-1))   ) -  ((sizeof(b)+sizeof(int)-1) & ~(sizeof(int)-1)) )))
#define va_end(a)

/* stuff to make the DICE code happy */
#define __stkargs __stdargs
#define __sigfunc void *
#define __geta4 __asm
#define __D0 register __d0
#include "nsignal.h"
#include "socket.h"

/* must be after the includes */
#define select(w,r,t,e,v) socket_select(w,r,t,e,v)
#endif
