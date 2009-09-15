#include "../lpc_incl.h"
#include "../file_incl.h"
#include "../network_incl.h"
#include "../socket_efuns.h"
#include "../include/socket_err.h"
#include "../main.h"

char *external_cmd[NUM_EXTERNAL_CMDS];

#ifdef F_EXTERNAL_START
int external_start (int which, svalue_t * args,
		svalue_t * arg1, svalue_t * arg2, svalue_t * arg3) {
	int sv[2];
	char *cmd;
	int fd;
	char **argv;
	pid_t ret;

	if (--which < 0 || which > (NUM_EXTERNAL_CMDS-1) || !external_cmd[which])
		error("Bad argument 1 to external_start()\n");
	cmd = external_cmd[which];
	fd = find_new_socket();
	if (fd < 0) return fd;

	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1)
		return EESOCKET;

	ret = fork();
	if (ret == -1) {
		error("fork() in external_start() failed: %s\n", strerror(errno));
	}
	if (ret) {
		close(sv[1]);
		lpc_socks[fd].fd = sv[0];
		lpc_socks[fd].flags = S_EXTERNAL;
		set_read_callback(fd, arg1);
		set_write_callback(fd, arg2);
		set_close_callback(fd, arg3);
		lpc_socks[fd].owner_ob = current_object;
		lpc_socks[fd].mode = STREAM;
		lpc_socks[fd].state = STATE_DATA_XFER;
		memset((char *) &lpc_socks[fd].l_addr, 0, sizeof(lpc_socks[fd].l_addr));
		memset((char *) &lpc_socks[fd].r_addr, 0, sizeof(lpc_socks[fd].r_addr));
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
		const char *p;
		char *arg;

		if (args->type == T_ARRAY) {
			n = args->u.arr->size;
		} else {
			p = args->u.string;

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
		}

		argv = CALLOCATE(n, char *, TAG_TEMPORARY, "external args");

		argv[0] = cmd;

		/* need writable version */
		if (args->type == T_ARRAY) {
			int j;
			svalue_t *sv = args->u.arr->item;

			for (j = 0; j < n; j++) {
				argv[i++] = alloc_cstring(sv[j].u.string, "external args");
			}
		} else {
			flag = 1;
			arg = alloc_cstring(args->u.string, "external args");
			while (*arg) {
				if (isspace(*arg)) {
					*arg = 0;
					flag = 1;
				} else {
					if (flag) {
						argv[i++] = arg;
						flag = 0;
					}
				}
				arg++;
			}
		}
		argv[i] = 0;

		close(sv[0]);
		for(i=0; i<5; i++)
			if(external_port[i].port)
				close(external_port[i].fd); //close external ports
		dup2(sv[1], 0);
		dup2(sv[1], 1);
		dup2(sv[1], 2);
		execv(cmd, argv);
		exit(0);
		return 0;
	}
}

void f_external_start (void)
{
	int fd, num_arg = st_num_arg;
	svalue_t *arg = sp - num_arg + 1;

	if (check_valid_socket("external", -1, current_object, "N/A", -1)) {
		fd = external_start(arg[0].u.number, arg + 1,
				arg + 2, arg + 3, (num_arg == 5 ? arg + 4 : 0));
		pop_n_elems(num_arg - 1);
		sp->u.number = fd;
	} else {
		pop_n_elems(num_arg - 1);
		sp->u.number = EESECURITY;
	}
}
#endif
