/*
 * Copyright (c) 1985, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)ftpd.c	based on 5.28	(Berkeley) 4/20/89";
#endif /* not lint */

/*
 * FTP server.
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <sys/dir.h>

#include <netinet/in.h>

#define	FTP_NAMES
#include <arpa/ftp.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>

#include <ctype.h>
#include <stdio.h>
#include <signal.h>
#include <pwd.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <strings.h>
#include <varargs.h>
#include "pathnames.h"
#include "config.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

#if defined(sun) && !defined(FD_SET)
/* FD_SET being defined is a cheap way of determining if we are on 4.0 or not */
typedef int uid_t;
typedef int gid_t;
#endif
/*
 * File containing login names
 * NOT to be used on this machine.
 * Commonly used to disallow uucp.
 */
extern	int errno;
extern	char *sys_errlist[];
extern	int sys_nerr;
extern	char *crypt();
extern	char version[];
extern	FILE *ftpd_popen(), *fopen(), *freopen();
extern	int  ftpd_pclose(), fclose();
extern	char *getline();
extern	char cbuf[];
extern	off_t restart_point;

struct	sockaddr_in ctrl_addr;
struct	sockaddr_in data_source;
struct	sockaddr_in data_dest;
struct	sockaddr_in his_addr;
struct	sockaddr_in pasv_addr;

int	data;
jmp_buf	errcatch, urgcatch;
int	logged_in;
struct	mudpw *pw;
int	debug;
int	timeout = 900;    /* timeout after 15 minutes of inactivity */
int	maxtimeout = 7200;/* don't allow idle time to be set beyond 2 hours */
int	logging = 1;
int	guest;
int	type;
int	form;
int	stru;			/* avoid C keyword */
int	mode;
int	usedefault = 1;		/* for data transfers */
int	pdata = -1;		/* for passive mode */
int	transflag;
off_t	file_size;
off_t	byte_count;
#if !defined(CMASK) || CMASK == 0
#undef CMASK
#define CMASK 027
#endif
int	defumask = CMASK;		/* default umask value */
char	tmpline[7];
char	hostname[MAXHOSTNAMELEN];
char	remotehost[MAXHOSTNAMELEN];

char *parse_path();
char *get_home();

/*
 * Timeout intervals for retrying connections
 * to hosts that don't accept PORT cmds.  This
 * is a kludge, but given the problems with TCP...
 */
#define	SWAITMAX	90	/* wait at most 90 seconds */
#define	SWAITINT	5	/* interval between retries */

int	swaitmax = SWAITMAX;
int	swaitint = SWAITINT;

int	lostconn();
int	myoob();
FILE	*getdatasock(), *dataconn();

main(argc, argv, envp)
	int argc;
	char *argv[];
	char **envp;
{
	int n, sockfd, newsockfd;
	struct	sockaddr_in tcp_srv_addr;
	int addrlen, on = 1;
	char *cp;
	int fd, wait_on_child();
	FILE *fp;

	if (chdir("/")) {
		fprintf(stderr, "error: can't chdir\n");
		exit(1);
	}
	switch(fork()) {
	case -1:
		fprintf(stderr, "error: can't fork\n");
		exit(1);
	case 0:
		break;
	default:
		exit(0);
	}
	if (setpgrp(0, getpid()) == -1) {
		fprintf(stderr, "error: can't change process group\n");
		exit(1);
	}
	if ((fd = open("/dev/tty", O_RDWR)) >= 0) {
		ioctl(fd, TIOCNOTTY, NULL);
		close(fd);
	}
	if ((freopen(FTPD_LOG, "a", stderr)) == NULL) {
		fprintf(stderr, "error: can't open log file\n");
		exit(1);
	}

	signal(SIGCLD, wait_on_child);
	bzero((char *) &tcp_srv_addr, sizeof(tcp_srv_addr));
	tcp_srv_addr.sin_family = AF_INET;
	tcp_srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	tcp_srv_addr.sin_port = htons(PORT);

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "can't create stream socket\n");
		exit(-1);
	}
	if (bind(sockfd, (struct sockaddr *) &tcp_srv_addr,
	  sizeof(tcp_srv_addr)) < 0) {
		fprintf(stderr, "can't bind local address\n");
		exit(-1);
	}
	listen(sockfd, 5);

main_again:
	addrlen = sizeof (his_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &his_addr, &addrlen);
	if (newsockfd < 0) {
		if (errno == EINTR)
			goto main_again;
		fprintf(stderr, "accept error\n");
		exit(-1);
	}

	switch(fork()) {
	case -1:
		fprintf(stderr, "server can't fork\n");
		exit(-1);
	case 0:
		dup2(newsockfd, 0);
		dup2(newsockfd, 1);
		for (n = 3; n < NOFILE; n++)
			close(n);
		break;
	default:
		close(newsockfd);
		goto main_again;
	}

	addrlen = sizeof (ctrl_addr);
	if (getsockname(0, (struct sockaddr *)&ctrl_addr, &addrlen) < 0) {
		log_it("getsockname (%s): %s",argv[0],sys_errlist[errno]);
		exit(1);
	}
	data_source.sin_port = htons(ntohs(ctrl_addr.sin_port) - 1);
	debug = 0;

	argc--, argv++;
	while (argc > 0 && *argv[0] == '-') {
		for (cp = &argv[0][1]; *cp; cp++) switch (*cp) {

		case 'v':
			debug = 1;
			break;

		case 'd':
			debug = 1;
			break;

		case 'l':
			logging = 1;
			break;

		case 't':
			timeout = atoi(++cp);
			if (maxtimeout < timeout)
				maxtimeout = timeout;
			goto nextopt;

		case 'T':
			maxtimeout = atoi(++cp);
			if (timeout > maxtimeout)
				timeout = maxtimeout;
			goto nextopt;

		case 'u':
		    {
			int val = 0;

			while (*++cp && *cp >= '0' && *cp <= '9')
				val = val*8 + *cp - '0';
			if (*cp)
				fprintf(stderr, "ftpd: Bad value for -u\n");
			else
				defumask = val;
			goto nextopt;
		    }

		default:
			fprintf(stderr, "ftpd: Unknown flag -%c ignored.\n",
			     *cp);
			break;
		}
nextopt:
		argc--, argv++;
	}
	(void) signal(SIGPIPE, lostconn);
	(void) signal(SIGCHLD, SIG_IGN);
	if ((int)signal(SIGURG, myoob) < 0)
		log_it("signal: %s",sys_errlist[errno]);

	/* handle urgent data inline */
	/* Sequent defines this, but it doesn't work */
#ifdef SO_OOBINLINE
	if (setsockopt(0, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)) < 0)
		log_it("setsockopt: %s",sys_errlist[errno]);
#endif
#ifdef	F_SETOWN
	if (fcntl(fileno(stdin), F_SETOWN, getpid()) == -1)
		log_it("fcntl F_SETOWN: %s",sys_errlist[errno]);
#endif
	dolog(&his_addr);
	/*
	 * Set up default state
	 */
	data = -1;
	type = TYPE_A;
	form = FORM_N;
	stru = STRU_F;
	mode = MODE_S;
	tmpline[0] = '\0';
	(void) gethostname(hostname, sizeof (hostname));
	reply(220, "%s LPmud FTP server (%s) ready.", hostname, version);
	(void) setjmp(errcatch);
	for (;;)
		(void) yyparse();
	/* NOTREACHED */
}

lostconn()
{

	if (debug)
		log_it("lost connection");
	dologout(-1);
}

/*
   getmudpw:

   This function gets info from the player.o file that is required by
   ftpd.  You may need to modify this function depending upon your
   mudlib and the variable names used in your player object.
*/

struct mudpw *
getmudpw(name)
char *name;
{
	FILE *fp;
	struct mudpw *save;
	char s[256], f;
	int ok;

	if ((save = (struct mudpw *) malloc(sizeof(struct mudpw))) == NULL)
		return (struct mudpw *)0;
	strcpy(save->pw_name, name);
	if (guest) {
		sprintf(save->pw_dir,"%s/ftp",MUD_PATH);
		return save;
	}
	f = name[0];
	get_player_fname(save,s);
	if ((fp = fopen(s, "r")) == NULL)
		return (struct mudpw *)0;
	ok = 0;
	while (ok != 2 && fgets(s, 255, fp) != NULL) {
                if (sscanf(s, "creator %d", &save->pw_level) == 1)
                  ok++;
                if (sscanf(s, "password \"%s\"", save->pw_passwd) == 1) {
                        save->pw_passwd[strlen(save->pw_passwd)-1] = '\0';
                        ok++;  
                }
	}
	if (ok == 2) {
		(void)get_home(save,save->pw_dir);
                save->pw_access = access_allowed(save);
		return save;
	}
	else {
		return NULL;
	}
}

int login_attempts;		/* number of failed login attempts */
int askpasswd;			/* had user command, ask for passwd */

/*
 * USER command.
 * Sets global passwd pointer pw if named account exists
 * and is acceptable; sets askpasswd if a PASS command is
 * expected. If logged in previously, need to reset state.
 * Disallow anyone mentioned in the file _PATH_FTPUSERS
 * to allow people such as root and uucp to be avoided.
 */
user(name)
	char *name;
{
	register char *cp;
	FILE *fd;
	char line[BUFSIZ];
	struct mudpw *getmudpw();

	if (logged_in) {
		end_login();
	}

	guest = anonymous(name);
	if ((pw = getmudpw(name)) != NULL) {
	    if (!guest) {
		if ((pw->pw_access == RESTRICTED) || pw->pw_level < 1) {
			reply(530, "User %s access denied.", name);
			if (logging)
				log_it("FTP LOGIN REFUSED FROM %s, %s",
				    remotehost, name);
			pw = (struct mudpw *) NULL;
			return;
		}
		if ((fd = fopen(_PATH_FTPUSERS, "r")) != NULL) {
			while (fgets(line, sizeof (line), fd) != NULL) {
				if ((cp = index(line, '\n')) != NULL)
					*cp = '\0';
				if (strcmp(line, name) == 0) {
					reply(530, "User %s access denied.", name);
					if (logging)
						log_it("FTP LOGIN REFUSED FROM %s, %s",
						    remotehost, name);
					pw = (struct mudpw *) NULL;
					return;
				}
		    }
		}
		(void) fclose(fd);
   	   }
	}
	askpasswd = 1;
	if (guest) {
		reply(331, "Guest login ok, type your email address as password.");
	}
	else {
		reply(331, "Password required for %s.", name);
	}
	/*
	 * Delay before reading passwd after first failed
	 * attempt to slow down passwd-guessing programs.
	 */
	if (login_attempts)
		sleep((unsigned) login_attempts);
}

/*
 * Terminate login as previous user, if any, resetting state;
 * used when USER command is given or login fails.
 */
end_login()
{
	pw = NULL;
	logged_in = 0;
	guest = 0;
}

pass(passwd)
char *passwd;
{
	char *xpasswd, *salt;

	if (logged_in || askpasswd == 0) {
		reply(503, "Login with USER first.");
		return;
	}
	if (pw == NULL)
		log_it("Oh dear");
	if (guest) {
		log_it("Guest login by %s.",passwd);
	}
	else {
		if (pw == NULL)
			salt = "xx";
		else
			salt = pw->pw_passwd;
		xpasswd = crypt(passwd, salt);
		/* The strcmp does not catch null passwords! */
		if (pw == NULL || *pw->pw_passwd == '\0' ||
		  strcmp(xpasswd, pw->pw_passwd)) {
			reply(530, "Login incorrect.");
			pw = NULL;
			if (debug)
				log_it("Got %s expected %s", xpasswd, pw->pw_passwd);
			if (login_attempts++ >= 5) {
				log_it("repeated login failures from %s",
				  remotehost);
				exit(0);
			}
			return;
		}
	}
	login_attempts = 0;		/* this time successful */

	logged_in = 1;
	if (chdir(pw->pw_dir) < 0) {
		if (chdir(MUD_PATH) < 0) {
			reply(530, "User %s: can't change directory to %s.",
			    pw->pw_name, pw->pw_dir);
			goto bad;
		} else
			lreply(230, "No directory! Logging in with eome=/");
	}
	if (guest)
		reply(230,"Guest login ok, access restrictions apply.");
	else
		reply(230, "User %s logged in.", pw->pw_name);
	if (logging)
		log_it("FTP LOGIN FROM %s, %s", remotehost, pw->pw_name);
	(void) umask(defumask);
	return;
bad:
	/* Forget all about it... */
	end_login();
}

retrieve(cmd, name)
	char *cmd, *name;
{
	FILE *fin, *dout;
	struct stat st;
	int (*closefunc)();
	char *newpath, *parse_path();

	if (cmd == 0) {
		newpath = parse_path(name);
		if (!valid_read(newpath)) {
			perror_reply(550, name);
			return;
		}
		fin = fopen(newpath, "r"), closefunc = fclose;
		st.st_size = 0;
	} else {
		char line[BUFSIZ];

		(void) sprintf(line, cmd, name), name = line;
		fin = ftpd_popen(line, "r"), closefunc = ftpd_pclose;
		st.st_size = -1;
		st.st_blksize = BUFSIZ;
	}
	if (fin == NULL) {
		if (errno != 0)
			perror_reply(550, name);
		return;
	}
	if (cmd == 0 &&
	    (fstat(fileno(fin), &st) < 0 || (st.st_mode&S_IFMT) != S_IFREG)) {
		reply(550, "%s: not a plain file.", name);
		goto done;
	}
	if (restart_point) {
		if (type == TYPE_A) {
			register int i, n, c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fin)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}	
		} else if (lseek(fileno(fin), restart_point, L_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	dout = dataconn(name, st.st_size, "w");
	if (dout == NULL)
		goto done;
	send_data(fin, dout, st.st_blksize);
	(void) fclose(dout);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fin);
}

store(name, mode, unique)
	char *name, *mode;
	int unique;
{
	FILE *fout, *din;
	struct stat st;
	int (*closefunc)();
	char *gunique();
	char *newpath, *parse_path();
	time_t t, time();
	char *ctime();

	newpath = parse_path(name);
	if (!valid_write(newpath)) {
		perror_reply(553, name);
		if (logging) {
			t = time((time_t *) 0);
			log_it("%s%s tried to upload %s", ctime(&t),
			  pw->pw_name, newpath+strlen(MUD_PATH));
		}
		return;
	}
	if (unique && stat(newpath, &st) == 0 &&
	    (newpath = gunique(newpath)) == NULL)
		return;

	if (restart_point)
		mode = "r+w";
	fout = fopen(newpath, mode);
	closefunc = fclose;
	if (fout == NULL) {
		perror_reply(553, name);
		return;
	}
	if (restart_point) {
		if (type == TYPE_A) {
			register int i, n, c;

			n = restart_point;
			i = 0;
			while (i++ < n) {
				if ((c=getc(fout)) == EOF) {
					perror_reply(550, name);
					goto done;
				}
				if (c == '\n')
					i++;
			}	
			/*
			 * We must do this seek to "current" position
			 * because we are changing from reading to
			 * writing.
			 */
			if (fseek(fout, 0L, L_INCR) < 0) {
				perror_reply(550, name);
				goto done;
			}
		} else if (lseek(fileno(fout), restart_point, L_SET) < 0) {
			perror_reply(550, name);
			goto done;
		}
	}
	din = dataconn(name, (off_t)-1, "r");
	if (din == NULL)
		goto done;
	if (receive_data(din, fout) == 0) {
		if (unique)
			reply(226, "Transfer complete (unique file name:%s).",
			    name);
		else
			reply(226, "Transfer complete.");
	}
	(void) fclose(din);
	data = -1;
	pdata = -1;
done:
	(*closefunc)(fout);
	if (logging && strncmp(newpath, pw->pw_dir, strlen(pw->pw_dir))) {
		t = time((time_t *) 0);
		log_it("%s%s uploaded %s", ctime(&t),
		  pw->pw_name, newpath+strlen(MUD_PATH));
	}
}

FILE *
getdatasock(mode)
	char *mode;
{
	int s, on = 1, tries;

	if (data >= 0)
		return (fdopen(data, mode));
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		return (NULL);
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR,
	    (char *) &on, sizeof (on)) < 0)
		goto bad;
	/* anchor socket to avoid multi-homing problems */
	data_source.sin_family = AF_INET;
	data_source.sin_addr = ctrl_addr.sin_addr;
	for (tries = 1; ; tries++) {
		if (bind(s, (struct sockaddr *)&data_source,
		    sizeof (data_source)) >= 0)
			break;
		if (errno != EADDRINUSE || tries > 10)
			goto bad;
		sleep(tries);
	}
	return (fdopen(s, mode));
bad:
	(void) close(s);
	return (NULL);
}

FILE *
dataconn(name, size, mode)
	char *name;
	off_t size;
	char *mode;
{
	char sizebuf[32];
	FILE *file;
	int retry = 0;

	file_size = size;
	byte_count = 0;
	if (size != (off_t) -1)
		(void) sprintf (sizebuf, " (%ld bytes)", size);
	else
		(void) strcpy(sizebuf, "");
	if (pdata >= 0) {
		struct sockaddr_in from;
		int s, fromlen = sizeof(from);

		s = accept(pdata, (struct sockaddr *)&from, &fromlen);
		if (s < 0) {
			reply(425, "Can't open data connection.");
			(void) close(pdata);
			pdata = -1;
			return(NULL);
		}
		(void) close(pdata);
		pdata = s;
		reply(150, "Opening %s mode data connection for %s%s.",
		     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
		return(fdopen(pdata, mode));
	}
	if (data >= 0) {
		reply(125, "Using existing data connection for %s%s.",
		    name, sizebuf);
		usedefault = 1;
		return (fdopen(data, mode));
	}
	if (usedefault)
		data_dest = his_addr;
	usedefault = 1;
	file = getdatasock(mode);
	if (file == NULL) {
		reply(425, "Can't create data socket (%s,%d): %s.",
		    inet_ntoa(data_source.sin_addr),
		    ntohs(data_source.sin_port),
		    errno < sys_nerr ? sys_errlist[errno] : "unknown error");
		return (NULL);
	}
	data = fileno(file);
	while (connect(data, (struct sockaddr *)&data_dest,
	    sizeof (data_dest)) < 0) {
		if (errno == EADDRINUSE && retry < swaitmax) {
			sleep((unsigned) swaitint);
			retry += swaitint;
			continue;
		}
		perror_reply(425, "Can't build data connection");
		(void) fclose(file);
		data = -1;
		return (NULL);
	}
	reply(150, "Opening %s mode data connection for %s%s.",
	     type == TYPE_A ? "ASCII" : "BINARY", name, sizebuf);
	return (file);
}

/*
 * Tranfer the contents of "instr" to
 * "outstr" peer using the appropriate
 * encapsulation of the data subject
 * to Mode, Structure, and Type.
 *
 * NB: Form isn't handled.
 */
send_data(instr, outstr, blksize)
	FILE *instr, *outstr;
	off_t blksize;
{
	register int c, cnt;
	register char *buf;
	int netfd, filefd;

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	switch (type) {

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n') {
				if (ferror(outstr))
					goto data_err;
				(void) putc('\r', outstr);
			}
			(void) putc(c, outstr);
		}
		fflush(outstr);
		transflag = 0;
		if (ferror(instr))
			goto file_err;
		if (ferror(outstr))
			goto data_err;
		reply(226, "Transfer complete.");
		return;

	case TYPE_I:
	case TYPE_L:
		if ((buf = (char *)malloc((u_int)blksize)) == NULL) {
			transflag = 0;
			perror_reply(451, "Local resource failure: malloc");
			return;
		}
		netfd = fileno(outstr);
		filefd = fileno(instr);
		while ((cnt = read(filefd, buf, (u_int)blksize)) > 0 &&
		    write(netfd, buf, cnt) == cnt)
			byte_count += cnt;
		transflag = 0;
		(void)free(buf);
		if (cnt != 0) {
			if (cnt < 0)
				goto file_err;
			goto data_err;
		}
		reply(226, "Transfer complete.");
		return;
	default:
		transflag = 0;
		reply(550, "Unimplemented TYPE %d in send_data", type);
		return;
	}

data_err:
	transflag = 0;
	perror_reply(426, "Data connection");
	return;

file_err:
	transflag = 0;
	perror_reply(551, "Error on input file");
}

/*
 * Transfer data from peer to
 * "outstr" using the appropriate
 * encapulation of the data subject
 * to Mode, Structure, and Type.
 *
 * N.B.: Form isn't handled.
 */
receive_data(instr, outstr)
	FILE *instr, *outstr;
{
	register int c;
	int cnt, bare_lfs = 0;
	char buf[BUFSIZ];

	transflag++;
	if (setjmp(urgcatch)) {
		transflag = 0;
		return (-1);
	}
	switch (type) {

	case TYPE_I:
	case TYPE_L:
		while ((cnt = read(fileno(instr), buf, sizeof buf)) > 0) {
			if (write(fileno(outstr), buf, cnt) != cnt)
				goto file_err;
			byte_count += cnt;
		}
		if (cnt < 0)
			goto data_err;
		transflag = 0;
		return (0);

	case TYPE_E:
		reply(553, "TYPE E not implemented.");
		transflag = 0;
		return (-1);

	case TYPE_A:
		while ((c = getc(instr)) != EOF) {
			byte_count++;
			if (c == '\n')
				bare_lfs++;
			while (c == '\r') {
				if (ferror(outstr))
					goto data_err;
				if ((c = getc(instr)) != '\n') {
					(void) putc ('\r', outstr);
					if (c == '\0' || c == EOF)
						goto contin2;
				}
			}
			(void) putc(c, outstr);
	contin2:	;
		}
		fflush(outstr);
		if (ferror(instr))
			goto data_err;
		if (ferror(outstr))
			goto file_err;
		transflag = 0;
		if (bare_lfs) {
			lreply(230, "WARNING! %d bare linefeeds received in ASCII mode");
			printf("   File may not have transferred correctly.\r\n");
		}
		return (0);
	default:
		reply(550, "Unimplemented TYPE %d in receive_data", type);
		transflag = 0;
		return (-1);
	}

data_err:
	transflag = 0;
	perror_reply(426, "Data Connection");
	return (-1);

file_err:
	transflag = 0;
	perror_reply(452, "Error writing file");
	return (-1);
}

statfilecmd(filename)
	char *filename;
{
	char line[BUFSIZ];
	FILE *fin;
	int c;

	(void) sprintf(line, "/bin/ls -lgA %s", filename);
	fin = ftpd_popen(line, "r");
	lreply(211, "status of %s:", filename);
	while ((c = getc(fin)) != EOF) {
		if (c == '\n') {
			if (ferror(stdout)){
				perror_reply(421, "control connection");
				(void) ftpd_pclose(fin);
				dologout(1);
				/* NOTREACHED */
			}
			if (ferror(fin)) {
				perror_reply(551, filename);
				(void) ftpd_pclose(fin);
				return;
			}
			(void) putc('\r', stdout);
		}
		(void) putc(c, stdout);
	}
	(void) ftpd_pclose(fin);
	reply(211, "End of Status");
}

statcmd()
{
	struct sockaddr_in *sin;
	u_char *a, *p;

	lreply(211, "%s FTP server status:", hostname, version);
	printf("     %s\r\n", version);
	printf("     Connected to %s", remotehost);
	if (isdigit(remotehost[0]))
		printf(" (%s)", inet_ntoa(his_addr.sin_addr));
	printf("\r\n");
	if (logged_in) {
		printf("     Logged in as %s\r\n", pw->pw_name);
	} else if (askpasswd)
		printf("     Waiting for password\r\n");
	else
		printf("     Waiting for user name\r\n");
	printf("     TYPE: %s", typenames[type]);
	if (type == TYPE_A || type == TYPE_E)
		printf(", FORM: %s", formnames[form]);
	if (type == TYPE_L)
#if NBBY == 8
		printf(" %d", NBBY);
#else
		printf(" %d", bytesize);	/* need definition! */
#endif
	printf("; STRUcture: %s; transfer MODE: %s\r\n",
	    strunames[stru], modenames[mode]);
	if (data != -1)
		printf("     Data connection open\r\n");
	else if (pdata != -1) {
		printf("     in Passive mode");
		sin = &pasv_addr;
		goto printaddr;
	} else if (usedefault == 0) {
		printf("     PORT");
		sin = &data_dest;
printaddr:
		a = (u_char *) &sin->sin_addr;
		p = (u_char *) &sin->sin_port;
#define UC(b) (((int) b) & 0xff)
		printf(" (%d,%d,%d,%d,%d,%d)\r\n", UC(a[0]),
			UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
#undef UC
	} else
		printf("     No data connection\r\n");
	reply(211, "End of status");
}

fatal(s)
	char *s;
{
	reply(451, "Error in server: %s\n", s);
	reply(221, "Closing connection due to server error.");
	dologout(0);
	/* NOTREACHED */
}

/* VARARGS2 */
reply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
	printf("%d ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
	printf("\r\n");
	(void)fflush(stdout);
	if (debug) {
		log_it("<--- %d ", n);
		log_it(fmt, p0, p1, p2, p3, p4, p5);
	}
}

/* VARARGS2 */
lreply(n, fmt, p0, p1, p2, p3, p4, p5)
	int n;
	char *fmt;
{
	printf("%d- ", n);
	printf(fmt, p0, p1, p2, p3, p4, p5);
	printf("\r\n");
	(void)fflush(stdout);
	if (debug) {
		log_it("<--- %d- ", n);
		log_it(fmt, p0, p1, p2, p3, p4, p5);
	}
}

ack(s)
	char *s;
{
	reply(250, "%s command successful.", s);
}

nack(s)
	char *s;
{
	reply(502, "%s command not implemented.", s);
}

/* ARGSUSED */
yyerror(s)
	char *s;
{
	char *cp;

	if (cp = index(cbuf,'\n'))
		*cp = '\0';
	reply(500, "'%s': command not understood.", cbuf);
}

delete(name)
	char *name;
{
	struct stat st;
	char *newpath, *parse_path();
	time_t t, time();
	char *ctime();

	newpath = parse_path(name);
	if (stat(newpath, &st) < 0) {
		perror_reply(550, name);
		return;
	}
	if (!valid_write(newpath)) {
		if (logging) {
			t = time((time_t *) 0);
			log_it("%s%s tried to delete %s", ctime(&t),
			  pw->pw_name, newpath+strlen(MUD_PATH));
		}
		perror_reply(550, name);
		return;
	}
	if ((st.st_mode&S_IFMT) == S_IFDIR) {
		if (rmdir(newpath) < 0) {
			perror_reply(550, name);
			return;
		}
		goto done;
	}
	if (unlink(newpath) < 0) {
		perror_reply(550, name);
		return;
	}
done:
	ack("DELE");
	if (logging && strncmp(newpath, pw->pw_dir, strlen(pw->pw_dir))) {
		t = time((time_t *) 0);
		log_it("%s%s deleted %s", ctime(&t),
		  pw->pw_name, newpath+strlen(MUD_PATH));
	}
}

cwd(path)
	char *path;
{
	char *newpath, *parse_path();

	newpath = parse_path(path);
	if (!valid_read(newpath) || chdir(newpath) < 0)
		perror_reply(550, path);
	else
		ack("CWD");
}

makedir(name)
	char *name;
{
	char *newpath, *parse_path();

	newpath = parse_path(name);
	if (!valid_write(newpath) || mkdir(name, 0777) < 0)
		perror_reply(550, name);
	else 
		reply(257, "MKD command successful.");
}

removedir(name)
	char *name;
{
	char *newpath, *parse_path();
	time_t t, time();
	char *ctime();

	newpath = parse_path(name);
	if (!valid_write(newpath) || rmdir(newpath) < 0)
		perror_reply(550, name);
	else {
		ack("RMD");
		if (logging && strncmp(newpath,
		  pw->pw_dir, strlen(pw->pw_dir))) {
			t = time((time_t *) 0);
			log_it("%s%s removed %s at %s", ctime(&t),
		  	pw->pw_name, newpath+strlen(MUD_PATH));
		}
	}
}

pwd()
{
	char path[MAXPATHLEN + 1], *sptr;
	extern char *getwd();
	register int len;

	len = strlen(MUD_PATH);
	if (getwd(path) == (char *)NULL || strlen(path) < len)
		reply(550, "%s.", path);
	else {
		sptr = path+len;
		if (*sptr == '\0')
			*(--sptr) = '/';
		reply(257, "\"%s\" is current directory.", sptr);
	}
}

char *
renamefrom(name)
	char *name;
{
	char *newpath, *parse_path();
	struct stat st;

	newpath = parse_path(name);
	if (!valid_read(newpath) || stat(newpath, &st) < 0) {
		perror_reply(550, name);
		return ((char *)0);
	}
	reply(350, "File exists, ready for destination name");
	return (name);
}

renamecmd(from, to)
	char *from, *to;
{
	char *np, temp[MAXPATHLEN+1];
	char *parse_path();
	time_t t, time();
	char *ctime();
	int len;

	strcpy(temp, parse_path(from));
	np = parse_path(to);
	if (!valid_write(temp) || !valid_write(np)) {
		if (logging) {
			len = strlen(MUD_PATH);
			t = time((time_t *) 0);
			log_it("%s%s tried to rename %s to %s", ctime(&t),
			  pw->pw_name, temp+len, np+len);
		}
		perror_reply(550, "rename");
	} else if (rename(temp, np) < 0)
		perror_reply(550, "rename");
	else {
		ack("RNTO");
		len = strlen(pw->pw_dir);
		if (logging && (strncmp(temp, pw->pw_dir, len) ||
		  strncmp(np, pw->pw_dir, len))) {
			len = strlen(MUD_PATH);
			t = time((time_t *) 0);
			log_it("%s%s renamed %s to %s", ctime(&t),
			  pw->pw_name, temp+len, np+len);
		}
	}
}

dolog(sin)
	struct sockaddr_in *sin;
{
	struct hostent *hp = gethostbyaddr((char *)&sin->sin_addr,
		sizeof (struct in_addr), AF_INET);
	time_t t, time();
	extern char *ctime();

	if (hp)
		(void) strncpy(remotehost, hp->h_name, sizeof (remotehost));
	else
		(void) strncpy(remotehost, inet_ntoa(sin->sin_addr),
		    sizeof (remotehost));

	if (logging) {
		t = time((time_t *) 0);
		log_it("connection from %s at %s",
		    remotehost, ctime(&t));
	}
}

dologout(status)
	int status;
{
	/* beware of flushing buffers after a SIGPIPE */
	_exit(status);
}

myoob()
{
	char *cp;

	/* only process if transfer occurring */
	if (!transflag)
		return;
	cp = tmpline;
	if (getline(cp, 7, stdin) == NULL) {
		reply(221, "You could at least say goodbye.");
		dologout(0);
	}
	upper(cp);
	if (strcmp(cp, "ABOR\r\n") == 0) {
		tmpline[0] = '\0';
		reply(426, "Transfer aborted. Data connection closed.");
		reply(226, "Abort successful");
		longjmp(urgcatch, 1);
	}
	if (strcmp(cp, "STAT\r\n") == 0) {
		if (file_size != (off_t) -1)
			reply(213, "Status: %lu of %lu bytes transferred",
			    byte_count, file_size);
		else
			reply(213, "Status: %lu bytes transferred", byte_count);
	}
}

/*
 * Note: a response of 425 is not mentioned as a possible response to
 * 	the PASV command in RFC959. However, it has been blessed as
 * 	a legitimate response by Jon Postel in a telephone conversation
 *	with Rick Adams on 25 Jan 89.
 */
passive()
{
	int len;
	register char *p, *a;

	pdata = socket(AF_INET, SOCK_STREAM, 0);
	if (pdata < 0) {
		perror_reply(425, "Can't open passive connection");
		return;
	}
	pasv_addr = ctrl_addr;
	pasv_addr.sin_port = 0;
	if (bind(pdata, (struct sockaddr *)&pasv_addr, sizeof(pasv_addr)) < 0) {
		goto pasv_error;
	}
	len = sizeof(pasv_addr);
	if (getsockname(pdata, (struct sockaddr *) &pasv_addr, &len) < 0)
		goto pasv_error;
	if (listen(pdata, 1) < 0)
		goto pasv_error;
	a = (char *) &pasv_addr.sin_addr;
	p = (char *) &pasv_addr.sin_port;

#define UC(b) (((int) b) & 0xff)

	reply(227, "Entering Passive Mode (%d,%d,%d,%d,%d,%d)", UC(a[0]),
		UC(a[1]), UC(a[2]), UC(a[3]), UC(p[0]), UC(p[1]));
	return;

pasv_error:
	(void) close(pdata);
	pdata = -1;
	perror_reply(425, "Can't open passive connection");
	return;
}

/*
 * Generate unique name for file with basename "local".
 * The file named "local" is already known to exist.
 * Generates failure reply on error.
 */
char *
gunique(local)
	char *local;
{
	static char new[MAXPATHLEN];
	struct stat st;
	char *cp = rindex(local, '/');
	int count = 0;

	if (cp)
		*cp = '\0';
	if (stat(cp ? local : ".", &st) < 0) {
		perror_reply(553, cp ? local : ".");
		return((char *) 0);
	}
	if (cp)
		*cp = '/';
	(void) strcpy(new, local);
	cp = new + strlen(new);
	*cp++ = '.';
	for (count = 1; count < 100; count++) {
		(void) sprintf(cp, "%d", count);
		if (stat(new, &st) < 0)
			return(new);
	}
	reply(452, "Unique file name cannot be created.");
	return((char *) 0);
}

/*
 * Format and send reply containing system error number.
 */
perror_reply(code, string)
	int code;
	char *string;
{
	if (errno < sys_nerr)
		reply(code, "%s: %s.", string, sys_errlist[errno]);
	else
		reply(code, "%s: unknown error %d.", string, errno);
}

static char *onefile[] = {
	"",
	0
};

send_file_list(whichfiles)
	char *whichfiles;
{
	struct stat st;
	DIR *dirp = NULL;
	struct direct *dir;
	FILE *dout = NULL;
	register char **dirlist, *dirname;
	int simple = 0;
	char *strpbrk();
	int len;
	char *cp;

	if (strpbrk(whichfiles, "{[*?") != NULL) {
		extern char **glob(), *globerr;

		globerr = NULL;
		dirlist = glob(whichfiles);
		if (globerr != NULL) {
			reply(550, globerr);
			return;
		} else if (dirlist == NULL) {
			errno = ENOENT;
			perror_reply(550, whichfiles);
			return;
		}
	} else {
		onefile[0] = whichfiles;
		dirlist = onefile;
		simple = 1;
	}

	if (setjmp(urgcatch)) {
		transflag = 0;
		return;
	}
	while (dirname = *dirlist++) {
		if (dirname[0] == '-' && *dirlist == NULL && !transflag) {
			retrieve("/bin/ls %s", dirname);
			return;
		}
		dirname = (char *)parse_path(dirname);
		if (!valid_read(dirname) || stat(dirname, &st) < 0) {
			perror_reply(550, whichfiles);
			if (dout != NULL) {
				(void) fclose(dout);
				transflag = 0;
				data = -1;
				pdata = -1;
			}
			return;
		}

		len = strlen(dirname) + 1;
		if ((st.st_mode&S_IFMT) == S_IFREG) {
			if (dout == NULL) {
				dout = dataconn("file list", (off_t)-1, "w");
				if (dout == NULL)
					return;
				transflag++;
			}
			cp = rindex(dirname, '/');
			fprintf(dout, "%s\n", ++cp);
			byte_count += strlen(cp) + 1;
			continue;
		} else if ((st.st_mode&S_IFMT) != S_IFDIR)
			continue;

		if ((dirp = opendir(dirname)) == NULL)
			continue;

		while ((dir = readdir(dirp)) != NULL) {
			char nbuf[MAXPATHLEN];

			if (dir->d_name[0] == '.' && dir->d_namlen == 1)
				continue;
			if (dir->d_name[0] == '.' && dir->d_name[1] == '.' &&
			    dir->d_namlen == 2)
				continue;

			sprintf(nbuf, "%s/%s", dirname, dir->d_name);

			/*
			 * We have to do a stat to insure it's
			 * not a directory or special file.
			 */
			if (simple || (stat(nbuf, &st) == 0 &&
			    (st.st_mode&S_IFMT) == S_IFREG)) {
				if (dout == NULL) {
					dout = dataconn("file list", (off_t)-1,
						"w");
					if (dout == NULL)
						return;
					transflag++;
				}
				fprintf(dout, "%s\n", nbuf+len);
				byte_count += strlen(nbuf)-len + 1;
			}
		}
		(void) closedir(dirp);
	}

	if (dout == NULL)
		reply(550, "No files found.");
	else if (ferror(dout) != 0)
		perror_reply(550, "Data connection");
	else
		reply(226, "Transfer complete.");

	transflag = 0;
	if (dout != NULL)
		(void) fclose(dout);
	data = -1;
	pdata = -1;
}

/*VARARGS1*/
log_it(fmt, a, b, c, d, e, f)
char *fmt;
{
	fprintf(stderr, fmt, a, b, c, d, e, f);
	fprintf(stderr, "\n");
	fflush(stderr);
}

char *
parse_path(path)
char *path;
{
	static char s_path[MAXPATHLEN + 1];
	static char tstr[MAXPATHLEN + 1];
	char a[MAXPATHLEN + 1];
	char *cp1, *cp2, *last, *current, f;
	extern char *getwd();
	struct mudpw *opw;
	int len;

	len = strlen(MUD_PATH);
	f = pw->pw_name[0];
	if ((*path == '/') && strncmp(path, MUD_PATH, len))
		sprintf(a, "%s%s", MUD_PATH, path);
	else if (sscanf(path, "~/%s", s_path) == 1)
		sprintf(a, "%s/%s", get_home(pw,tstr), s_path);
	else if (sscanf(path, "~%s", s_path) == 1) {
		sscanf(s_path,"%[^/]",tstr);
		opw = getmudpw(tstr);
		if (opw) {
			sprintf(a, "%s/%s", get_home(opw,tstr), s_path);
		}
		else {
			sprintf(a, "%s/%s", MUD_PATH, path);
		}
		free(opw);
	}
	else if (*path == '~')
		(void)get_home(pw,a);
	else {
		if (getwd(s_path) == NULL)
			sprintf(a, "%s/%s", MUD_PATH, path);
		else
			sprintf(a, "%s/%s", s_path, path);
	}
	if (strncmp(a, MUD_PATH, len-1))
		strcpy(a, MUD_PATH);
	cp1 = a;
	cp2 = s_path;
	while(*cp1) {
		switch (*cp1) {
		case '.':
			if (*++cp1 == '.') {
				switch (*(cp1+1)) {
				case '\0':
				case '/':
					if (last < s_path+len) {
						cp2 = s_path+len;
					} else {
						cp2 = last;
						*cp2 = '\0';
						current = rindex(s_path, "/");
					}
					break;
				case '.':
					while (*++cp1 == '.')
						*cp2++ = *cp1;
					cp1--;
				default:
					*cp2++ = *cp1;
					*cp2++ = *cp1;
				}
			} else if (*cp1 == '\0') {
				cp1--;
				cp2--;
			} else if (*cp1 != '/') {
				*cp2++ = '.';
				*cp2++ = *cp1;
			}
			break;
		case '/':
			if (*(cp2-1) == '/')
				break;
			last = current;
			current = cp2;
		default:
			*cp2++ = *cp1;
		}
		cp1++;
	}
	*cp2 = 0;
	return s_path;
}

wait_on_child()
{
	union wait status;

	while (wait3(&status, WNOHANG, (struct rusage *) 0) > 0)
		;
}
