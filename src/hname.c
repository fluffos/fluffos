#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

main()
{
    char buf[80];
    long addr;
    struct hostent *hp;

/*fprintf(stderr, "hname starts\n");*/
    printf("\n");
    fflush(stdout);
    for(;;) {
	if (gets(buf) == NULL)
	    break;
/*fprintf(stderr, "hname got %s\n", buf);*/
	addr = inet_addr(buf);
	if (addr != -1) {
	    hp = gethostbyaddr((char *)&addr, 4, AF_INET);
	    if (!hp) {
		sleep(5);
	        hp = gethostbyaddr((char *)&addr, 4, AF_INET);
	    }
	    if (hp) {
/*fprintf(stderr, "hname sends %s...", hp->h_name);*/
		printf("%s %s\n", buf, hp->h_name);
		fflush(stdout);
/*fprintf(stderr, "done\n");*/
	    }
	}
    }
/*fprintf(stderr, "hname exits\n");*/
}
