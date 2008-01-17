/* portbind.c: Tim Hollebeek, Oct 28, 1996 */

#include "std.h"
#include "network_incl.h"

#define HANDLE_ERROR(routine, call) if ((call) == -1) { perror(#routine); exit(-1); }

int main(int argc, char **argv) {
#ifndef WIN32
    int port = -1;
    char *ipaddress = 0;
    char *driver_name = "./driver";
    int uid = -1;
    int gid = -1;
    
    int fd;
    int optval = 1;
    struct sockaddr_in sin;
    
    /*Args are passed down to the driver, but we process the following options:
     *
     * -p <port number>
     * -d <driver path>
     * -u <uid>
     * -g <gid>
     * -i <ip address>
     * 
     * anything else causes us to quit scanning options.
     */
    while (argc > 1) {
	if (strcmp(argv[1], "-p") == 0) {
	    if (argc == 2 || sscanf(argv[2], "%d", &port) != 1) {
		fprintf(stderr, "%s: -p must be followed by a port number.\n",
			argv[0]);
		exit(-1);
	    }
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[1], "-d") == 0) {
	    if (argc == 2) {
		fprintf(stderr, "%s: -d must be followed by the driver pathname.\n", argv[0]);
		exit(-1);
	    }
	    driver_name = argv[2];
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[1], "-u") == 0) {
	    if (argc == 2 || sscanf(argv[2], "%d", &uid) != 1) {
		fprintf(stderr, "%s: -u must be followed by a valid numeric uid.\n", argv[0]);
		exit(-1);
	    }
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[1], "-g") == 0) {
	    if (argc == 2 || sscanf(argv[2], "%d", &gid) != 1) {
		fprintf(stderr, "%s: -g must be followed by a valid numeric gid.\n", argv[0]);
		exit(-1);
	    }
	    argc -= 2;
	    argv += 2;
	} else if (strcmp(argv[1], "-i") == 0) {
	    if (argc == 2 || inet_addr(argv[2]) == INADDR_NONE) {
		fprintf(stderr, "%s: -i must be followed by a valid dotted decimal ip address.\n", argv[0]);
		exit(-1);
	    }
	    ipaddress = argv[2];
	    argc -= 2;
	    argv += 2;
	} else break;
    }
    
    if (port == -1) {
	fprintf(stderr, "%s: a port must be specified with -p.\n", argv[0]);
	exit(-1);
    }

    /* create the socket */
    HANDLE_ERROR(socket, fd = socket(AF_INET, SOCK_STREAM, 0));

    /* move to fd #6 */
    HANDLE_ERROR(dup2, dup2(fd, 6));

    /* close the old copy */
    HANDLE_ERROR(close, close(fd));
    
    /* set local address reuse */
    HANDLE_ERROR(setsockopt, setsockopt(6, SOL_SOCKET, SO_REUSEADDR, (char *)&optval, sizeof(optval)) == -1);

    /* setup our address */
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = (ipaddress ? inet_addr(ipaddress) : INADDR_ANY);
    sin.sin_port = htons((unsigned short)port);

    /* bind to our address */
    HANDLE_ERROR(bind, bind(6, (struct sockaddr *)&sin, sizeof(sin)));

    /* change our uid and gid */
    if (gid != -1) 
	HANDLE_ERROR(setgid, setgid(gid));

    if (uid != -1) 
	HANDLE_ERROR(setuid, setuid(uid));

    argv[0] = driver_name;
    /* exec the driver */
    HANDLE_ERROR(execv, execv(driver_name, argv));
#endif
    return 0;
}
