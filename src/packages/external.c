#include "../lpc_incl.h"
#include "../file_incl.h"
#include "../network_incl.h"
#include "../socket_efuns.h"
#include "../include/socket_err.h"

char *external_cmd[5];

#ifdef F_EXTERNAL_START
int external_start P5(int, which, char *, args,
		      svalue_t *, arg1, svalue_t *, arg2, svalue_t *, arg3) {
    int sv[2];
    char *cmd;
    int fd;
    char **argv;
    
    if (--which < 0 || which > 4 || !external_cmd[which])
	error("Bad argument 1 to external_start()\n");
    cmd = external_cmd[which];
    fd = find_new_socket();
    if (fd < 0) return fd;
    
    if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1)
	return EESOCKET;
 
    if (fork()) {
	close(sv[1]);
	lpc_socks[fd].fd = sv[0];
	lpc_socks[fd].flags = S_EXTERNAL;
	set_read_callback(fd, arg1);
	set_write_callback(fd, arg2);
	set_close_callback(fd, arg3);
	lpc_socks[fd].owner_ob = current_object;
	lpc_socks[fd].mode = STREAM;
	lpc_socks[fd].state = DATA_XFER;
	memset((char *) &lpc_socks[fd].l_addr, 0, sizeof(lpc_socks[fd].l_addr));
	memset((char *) &lpc_socks[fd].r_addr, 0, sizeof(lpc_socks[fd].r_addr));
	lpc_socks[fd].name[0] = '\0';
	lpc_socks[fd].owner_ob = current_object;
	lpc_socks[fd].release_ob = NULL;
	lpc_socks[fd].r_buf = NULL;
	lpc_socks[fd].r_off = 0;
	lpc_socks[fd].r_len = 0;
	lpc_socks[fd].w_buf = NULL;
	lpc_socks[fd].w_off = 0;
	lpc_socks[fd].w_len = 0;

	current_object->flags |= O_EFUN_SOCKET;
	return fd;
    } else {
	int flag = 1;
	int i = 1;
	int n = 1;
	char *p;
	
	p = args;
	while (*p) {
	    if (isspace(*p)) {
		flag = 1;
	    } else {
		if (flag) {
		    n++;
		    flag = 0;
		}
	    }
	    p++;
	}

	argv = CALLOCATE(n, char *, TAG_TEMPORARY, "external args");

	argv[0] = cmd;

	/* need writable version */
	args = alloc_cstring(args, "external args");
	while (*args) {
	    if (isspace(*args)) {
		*args = 0;
		flag = 1;
	    } else {
		if (flag) { argv[i++] = args; flag = 0; }
	    }
	    args++;
	}
	argv[i] = 0;
	
	close(sv[0]);
	dup2(sv[1], 0);
	dup2(sv[1], 1);
	dup2(sv[1], 2);
	execv(cmd, argv);
	return 0;
    }
}

void f_external_start PROT((void))
{
    int fd, num_arg = st_num_arg;
    svalue_t *arg = sp - num_arg + 1;
    
    if (check_valid_socket("external", -1, current_object, "N/A", -1)) {
	fd = external_start(arg[0].u.number, arg[1].u.string,
			    arg + 2, arg + 3, (num_arg == 5 ? arg + 4 : 0));
	pop_n_elems(num_arg - 1);
	sp->u.number = fd;
    } else {
	pop_n_elems(num_arg - 1);
	sp->u.number = EESECURITY;
    }
}
#endif
