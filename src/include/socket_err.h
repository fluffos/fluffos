/*
 * socket_errors.h -- definitions for efun socket error return codes.
 *    5-92 : Dwayne Fontenot (Jacques@TMI) : original coding.
 *   10-92 : Dave Richards (Cynosure) : less original coding.
 */

#ifndef _SOCKET_ERRORS_H_
#define _SOCKET_ERRORS_H_

#define EESUCCESS	  1	/* Call was successful */
#define EESOCKET	 -1	/* Problem creating socket */
#define EESETSOCKOPT	 -2	/* Problem with setsockopt */
#define	EENONBLOCK	 -3	/* Problem setting non-blocking mode */
#define EENOSOCKS	 -4	/* UNUSED */
#define EEFDRANGE	 -5	/* Descriptor out of range */
#define EEBADF		 -6	/* Descriptor is invalid */
#define EESECURITY	 -7	/* Security violation attempted */
#define	EEISBOUND	 -8	/* Socket is already bound */
#define EEADDRINUSE	 -9	/* Address already in use */
#define EEBIND		-10	/* Problem with bind */
#define EEGETSOCKNAME	-11	/* Problem with getsockname */
#define EEMODENOTSUPP	-12	/* Socket mode not supported */
#define EENOADDR	-13	/* Socket not bound to an address */
#define EEISCONN	-14	/* Socket is already connected */
#define EELISTEN	-15	/* Problem with listen */
#define EENOTLISTN	-16	/* Socket not listening */
#define EEWOULDBLOCK	-17	/* Operation would block */
#define EEINTR		-18	/* Interrupted system call */
#define EEACCEPT	-19	/* Problem with accept */
#define	EEISLISTEN	-20	/* Socket is listening */
#define EEBADADDR	-21	/* Problem with address format */
#define EEALREADY	-22	/* Operation already in progress */
#define EECONNREFUSED	-23	/* Connection refused */
#define EECONNECT	-24	/* Problem with connect */
#define EENOTCONN	-25	/* Socket not connected */
#define EETYPENOTSUPP	-26	/* Object type not supported */
#define	EESENDTO	-27	/* Problem with sendto */
#define	EESEND		-28	/* Problem with send */
#define	EECALLBACK	-29	/* Wait for callback */
#define EESOCKRLSD	-30	/* Socket already released */
#define EESOCKNOTRLSD   -31	/* Socket not released */
#define EEBADDATA       -32	/* sending data with too many nested levels */

#define	ERROR_STRINGS	 33	/* sizeof (error_strings) */

#endif				/* _SOCKET_ERRORS_H_ */
