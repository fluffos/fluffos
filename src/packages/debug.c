void f_thread() {

	int sv[2];
	fd = find_new_socket();
	    if (fd < 0) 
	    	return fd;
	if (socketpair(PF_UNIX, SOCK_STREAM, 0, sv) == -1)
		return EESOCKET;
	ret = fork();
	if (ret == -1) {
		error("fork() in debug() failed: %s\n", strerror(errno));
	}
	if(ret){
		close(sv[1]);
        lpc_socks[fd].fd = sv[0];
        lpc_socks[fd].flags = S_EXTERNAL;
        set_read_callback(fd, sp-3);
        set_write_callback(fd, sp-2);
        set_close_callback(fd, sp-1);
        lpc_socks[fd].owner_ob = current_object;
        lpc_socks[fd].mode = MUD;
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
	}
	close(sv[0]);
	function_to_call_t cb;
	memset(&cb, 0, sizeof(function_to_call_t));
	process_efun_callback(0, &cb, F_THREAD);
	for(i=0; i<5; i++)
	  if(external_port[i].port)
	    close(external_port[i].fd); //close external ports
	for(i=0;i<sizeof(lpc_socks)/sizeof(lpc_socks[0]);i++)
		close(lpc_sock[i].fd);
	svalue_t *res = call_efun_callback(&cb, 1);
	switch (res->type) {

	        case T_OBJECT:
	            break;
	        default:
	            save_svalue_depth = 0;
	            int len = svalue_save_size(message);
	            if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
	            	OS_socket_write(sv[1], "\x00\x00\x00\x11\"result too big\"", 21);
	            	break;
	            }
	            char *buf = (char *)
	                DMALLOC(len + 5, TAG_TEMPORARY, "socket_write: default");
	            if (buf == NULL)
	                break;
	            *(INT_32 *) buf = htonl((long) len);
	            len += 4;
	            buf[4] = '\0';
	            p = buf + 4;
	            save_svalue(message, &p);
	            int ret,written = 0;
	            while(written < len){
	            	ret = OS_socket_write(sv[1], buf+written, len-written);
	            	if(ret < 0)
	            		break;
	            	written += ret;
	            }
	            break;
	        }
	fflush(0);
	exit(0);
}
