/*
 * Amiga include file for MudOS
 *   By Wildcard (umfehr06@ccu.umanitoba.ca)
 */

#ifndef AMIGA_AMIGA_H
#define AMIGA_AMIGA_H

/* file.c */
#define _IFIFO  0010000
#define _IFCHR  0020000
#define _IFBLK  0060000
#define _IFLNK  0120000
#define S_IFCHR _IFCHR
#define S_IFBLK _IFBLK
#define S_IFIFO _IFIFO
#define S_IFLNK _IFLNK

/* comm.c, file.c, backend.c, main.c, socket_efuns.c, addr_server.c */
#define NBBY    8
#define howmany(x, y)   (((x)+((y)-1))/(y))
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))

/* comm.c */
#define TELOPT_ECHO 1
#define TELOPT_SGA  3
#define TELOPT_TM   6
#define EOR     239
#define SE      240
#define NOP     241
#define DM  	242
#define BREAK   243
#define IP      244
#define AO      245
#define AYT     246
#define EC      247
#define EL      248
#define GA      249
#define SB      250
#define WILL    251
#define WONT    252
#define DO  	253
#define DONT    254
#define IAC 	255
#define MSG_OOB     0x1

/* mudlib_stats.c */
#define MAXPATHLEN   1024

/* socket_efuns.c */
#define EINPROGRESS  36
#define EALREADY     37
#define ECONNREFUSED 61


/*
 * broken c.lib routine missing support for permissions
 */
#define mkdir(a,b) mkdir(a)

/*
 * missing from the "standard" include files
 */
int gethostname PROT((char *, int));

#endif
