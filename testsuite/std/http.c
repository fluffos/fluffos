#define EESUCCESS 1       /* Call was successful */
#define EESOCKET -1       /* Problem creating socket */
#define EESETSOCKOPT -2   /* Problem with setsockopt */
#define EENONBLOCK -3     /* Problem setting non-blocking mode */
#define EENOSOCKS -4      /* UNUSED */
#define EEFDRANGE -5      /* Descriptor out of range */
#define EEBADF -6         /* Descriptor is invalid */
#define EESECURITY -7     /* Security violation attempted */
#define EEISBOUND -8      /* Socket is already bound */
#define EEADDRINUSE -9    /* Address already in use */
#define EEBIND -10        /* Problem with bind */
#define EEGETSOCKNAME -11 /* Problem with getsockname */
#define EEMODENOTSUPP -12 /* Socket mode not supported */
#define EENOADDR -13      /* Socket not bound to an address */
#define EEISCONN -14      /* Socket is already connected */
#define EELISTEN -15      /* Problem with listen */
#define EENOTLISTN -16    /* Socket not listening */
#define EEWOULDBLOCK -17  /* Operation would block */
#define EEINTR -18        /* Interrupted system call */
#define EEACCEPT -19      /* Problem with accept */
#define EEISLISTEN -20    /* Socket is listening */
#define EEBADADDR -21     /* Problem with address format */
#define EEALREADY -22     /* Operation already in progress */
#define EECONNREFUSED -23 /* Connection refused */
#define EECONNECT -24     /* Problem with connect */
#define EENOTCONN -25     /* Socket not connected */
#define EETYPENOTSUPP -26 /* Object type not supported */
#define EESENDTO -27      /* Problem with sendto */
#define EESEND -28        /* Problem with send */
#define EECALLBACK -29    /* Wait for callback */
#define EESOCKRLSD -30    /* Socket already released */
#define EESOCKNOTRLSD -31 /* Socket not released */
#define EEBADDATA -32     /* sending data with too many nested levels */
#define WRITE_WAIT_CALLBACK 0
#define WRITE_GO_AHEAD      1

#define STREAM 1
#define DATAGRAM 2

#define STATE_RESOLVING 0
#define STATE_CONNECTING 1
#define STATE_CLOSED 2
#define STATE_CONNECTED 3

nosave mapping hostname_to_fd;
nosave mapping status;
nosave object receiver;

void create() {
  hostname_to_fd = ([]);
  status = ([]);
}

void socket_shutdown(int fd) {
  status[fd]["status"] = STATE_CLOSED;

  tell_object(receiver, sprintf("%O", status));
}

void receive_data(int fd, mixed result) {
    tell_object(receiver, sprintf("fd %d received: %O", fd, result));
}

void write_data(int fd) {
  status[fd]["status"] = STATE_CONNECTED;

  tell_object(receiver, sprintf("%O", status));

  socket_write(fd, "GET " + status[fd]["path"] + " HTTP/1.0\nHost: " + status[fd]["host"] + "\n\r\n\r");
}

void on_resolve(string host, string addr, int key) {
  int fd;
  int ret;

  tell_object(receiver, sprintf("%s: %s\n", host, addr));

  if (addr)
  {
    fd = hostname_to_fd[host];

    ret = socket_connect(fd, addr + " " + status[fd]["port"], "receive_data", "write_data");
    if(ret != EESUCCESS){
      tell_object(receiver, "unable to connect: " + socket_error(ret) + "\n");
      socket_close(fd);
      return ;
    }

    status[fd]["status"] = STATE_CONNECTING;
    tell_object(receiver, sprintf("%O", socket_status(fd)));
  }
}


int get(string host, int port, string path) {
  int ret;
  int fd;

  receiver = this_player();

  fd = socket_create(STREAM, "receive_data", "socket_shutdown");
  status[fd] = ([]);
  status[fd]["status"] = STATE_RESOLVING;
  status[fd]["host"] = host;
  status[fd]["port"] = port;
  status[fd]["path"] = path;

  tell_object(receiver, sprintf("%O", status));
  hostname_to_fd[host] = fd;
  resolve(host, "on_resolve");
  return 0;
}
