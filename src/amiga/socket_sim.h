/* hosts/amiga/socket_sim.h */

#ifndef SOCKET_SIM_H
#define SOCKET_SIM_H

#define FD_SETSIZE      256

/* stuff used in socket_ctrl.c */
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

#define EMSGSIZE     30
#define ENETUNREACH  31
#define EHOSTUNREACH 32
#define EADDRINUSE   33
#define ETIMEDOUT    34
#define ECONNRESET   35
#define ENOBUFS      36

#define SOCK_STREAM 0
#define SOCK_DGRAM 1

#define SOL_SOCKET 0
#define SO_REUSEADDR 0

#define AF_INET 	2	/* internetwork: UDP, TCP, etc. */

typedef short SOCKET_T;

#define INADDR_ANY 0

#define htons(n) (n)

#define socket_number(s) (s)

struct in_addr {
    long s_addr;
};

 /* sizeof(sockaddr_in) == sizeof(sockaddr) ! */

struct sockaddr_in {
    short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

struct sockaddr {
    unsigned short sa_family;	/* address family */
    char sa_data[14];		/* up to 14 bytes of direct address */
};

struct hostent {
    char *h_name;		/* official name of host */
    char **h_aliases;		/* alias list */
    int h_addrtype;		/* host address type */
    int h_length;		/* length of address */
    char **h_addr_list;		/* list of addresses from name server */
#define h_addr	h_addr_list[0]	/* address, for backward compatiblity */
};

#define setsockopt(s,a,b,c,d) (0)
#define listen(socket,queue_size) (0)
#define socket_ioctl(socket,code,p) (0)
#define ntohl(x) (x)

#ifndef LATTICE
#define recvfrom(s,b,c,f,a,l) (-1)
#define sendto(s,m,n,f,a,l) (-1)
#endif
extern int shutdown(int, int);
extern int write_socket(int, char *, int);
extern int read_socket(int, char *, int);
extern int find_free_socket();
extern int socket(int, int, int);
extern int accept(int, struct sockaddr *, int *);
extern int close_socket(int);
extern int bind(int, struct sockaddr *, int);
extern int socket_select(int, fd_set *, fd_set *, fd_set *
			    ,struct timeval *);
    extern struct hostent *gethostbyname(char *);
    extern int gethostname(char *, int);
    extern char *inet_ntoa(struct in_addr);
    extern unsigned long inet_addr(char *);
    extern int getpeername(int, struct sockaddr *, int *);

#define read(s,t,l) read_socket(s,t,l)
#define write(s,t,l) write_socket(s,t,l)
#define close(s) close_socket(s)
#define select(w,r,t,e,v) socket_select(w,r,t,e,v)

#endif				/* SOCKET_SIM_H */
