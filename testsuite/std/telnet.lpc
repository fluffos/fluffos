// written by Dwayne Fontenot (Jacques)
// last modified: 1992 October 19 (runs on the Basis mudlib)

// This object implements a telnet client (providing a subset of the telnet
// protocol) using STREAM mode of MudOS 0.9 LPC sockets.  See the init()
// function // to find out the commands this terminal understands.
// This object may be used from within a MudOS mud to connect to any
// networked server that understands the telnet protocol (including
// another LPmud).

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

#define IAC  255
#define DONT 254
#define DO   253
#define WONT 252
#define WILL 251

#define TELOPT_ECHO   1
#define TELOPT_SGA    3
#define TELOPT_TTYPE 24
#define TELOPT_NAWS  31

nosave string *telopts = ({"BINARY", "ECHO", "RCP", "SUPPRESS GO AHEAD",
      "NAME", "STATUS", "TIMING MARK", "RCTE", "NAOL", "NAOP",
      "NAOCRD", "NAOHTS", "NAOHTD", "NAOFFD", "NAOVTS",
      "NAOVTD", "NAOLFD", "EXTEND ASCII", "LOGOUT", "BYTE MACRO",
      "DATA ENTRY TERMINAL", "SUPDUP", "SUPDUP OUTPUT",
      "SEND LOCATION", "TERMINAL TYPE", "END OF RECORD",
      "TACACS UID", "OUTPUT MARKING", "TTYLOC",
      "3270 REGIME", "X.3 PAD", "NAWS", "TSPEED", "LFLOW",
      "LINEMODE"});

nosave string s_iac_dont_echo;
nosave string s_iac_do_echo;
nosave string s_iac_wont_echo;
nosave string s_iac_will_echo;
nosave string s_iac_dont_sga;
nosave string s_iac_do_sga;
nosave string s_iac_wont_sga;
nosave string s_iac_will_sga;
nosave string s_iac_wont_ttype;
nosave string s_iac_wont_naws;
nosave string s_iac;
nosave string s_dont_echo;
nosave string s_do_echo;

private string callback;

nosave private int conn_fd = 0;
nosave private int connected = 0;
nosave private int verbose = 0;

private int write_state = WRITE_WAIT_CALLBACK;
private string write_message = "";

protected void init_tel_neg()
{
  s_iac_dont_echo  = sprintf("%c%c%c",IAC,DONT,TELOPT_ECHO);
  s_iac_do_echo    = sprintf("%c%c%c",IAC,DO  ,TELOPT_ECHO);
  s_iac_wont_echo  = sprintf("%c%c%c",IAC,WONT,TELOPT_ECHO);
  s_iac_will_echo  = sprintf("%c%c%c",IAC,WILL,TELOPT_ECHO);
  s_iac_dont_sga   = sprintf("%c%c%c",IAC,DONT,TELOPT_SGA);
  s_iac_do_sga     = sprintf("%c%c%c",IAC,DO  ,TELOPT_SGA);
  s_iac_wont_sga   = sprintf("%c%c%c",IAC,WONT,TELOPT_SGA);
  s_iac_will_sga   = sprintf("%c%c%c",IAC,WILL,TELOPT_SGA);
  s_iac_wont_ttype = sprintf("%c%c%c",IAC,WONT,TELOPT_TTYPE);
  s_iac_wont_naws  = sprintf("%c%c%c",IAC,WONT,TELOPT_NAWS);
  s_iac            = sprintf("%c",    IAC);
  s_dont_echo      = sprintf("%c%c",  DONT,TELOPT_ECHO);
  s_do_echo        = sprintf("%c%c",  DO,  TELOPT_ECHO);
}

void
set_callback(string arg)
{
  callback = arg;
}

void create()
{
  init_tel_neg();
  connected = 0;
  verbose = 0;
  set_callback("handler");
}

void
set_verbosity(int v)
{
  verbose = v;
}

int
query_connected()
{
  return connected;
}

void disconnected()
{
  call_other(this_object(), callback, "close");
  connected = 0;
}

int connected()
{
  call_other(this_object(), callback, "open");
  connected = 1;
}

void my_socket_write(int fd, string message)
{
  int ret;

  write_message = write_message + message;
  if(write_state == WRITE_GO_AHEAD){
    ret = socket_write(fd, write_message);
    write_message = "";
    if(ret == EESUCCESS) write_state = WRITE_GO_AHEAD;
    else if(ret == EECALLBACK) write_state = WRITE_WAIT_CALLBACK;
  }
}

int line(string str)
{
  if(connected){
    my_socket_write(conn_fd,s_iac_dont_sga+s_iac_dont_echo);
    write("SENT dont SUPPRESS GO AHEAD\nSENT dont ECHO\n");
    return(1);
  }
  return(0);
}

int char(string str)
{
if(connected){
my_socket_write(conn_fd,s_iac_do_sga+s_iac_do_echo);
write("SENT do SUPPRESS GO AHEAD\nSENT do ECHO\n");
return(1);
}
return(0);
}

int connect(string str)
{
  int ret;

  if(!str) return(0);
  conn_fd = socket_create(STREAM,"socket_shutdown");
  ret = socket_connect(conn_fd,str,"receive_data","write_data");
  if(ret != EESUCCESS){
    write("unable to connect: " + socket_error(ret) + "\n");
    return(0);
  }
  connected();
  return(1);
}

int send(string str)
{
  if(connected){
    if(!str){
      write("Sending CR.\n");
      my_socket_write(conn_fd,"\n");
      return(1);
    }
    my_socket_write(conn_fd,str + "\n");
    return(1);
  }
  return(0);
}

int disconnect(string str)
{
  int ret;

  ret = socket_close(conn_fd);
  if(ret <= 0){
    notify_fail("unable to disconnect.\n");
    return(0);
  }
  disconnected();
  return(1);
}

void receive_data(int rec_fd,string msg)
{
  string *chunks;
  int i;
  object hearer;

  hearer = environment(this_object());
  chunks = explode(msg,s_iac);
  for(i=0;i<sizeof(chunks);i++){
    switch(chunks[i][0]){
      case DONT:
        if (verbose)
          message("telnet","RCVD dont "+telopts[chunks[i][1]]+"\n",hearer,
                  this_object());
        switch(chunks[i][1]){
          case TELOPT_ECHO:
            message("",s_iac_dont_echo,hearer,this_object());
            break;
        }
        if(strlen(chunks[i]) > 2)
          message("telnet",chunks[i][2..strlen(chunks[i])-1],hearer,
                  this_object());
        break;
      case DO:
        if (verbose)
          message("telnet","RCVD do "+telopts[chunks[i][1]]+"\n",hearer,
                  this_object());
        switch(chunks[i][1]){
          case TELOPT_ECHO:
            my_socket_write(rec_fd,s_iac_wont_echo);
            if (verbose)
              message("telnet","SENT wont ECHO\n",hearer,this_object());
            break;
          case TELOPT_TTYPE:
            my_socket_write(rec_fd,s_iac_wont_ttype);
            if (verbose)
              message("telnet","SENT wont TERMINAL TYPE\n",hearer,this_object());
            break;
          case TELOPT_NAWS:
            my_socket_write(rec_fd,s_iac_wont_naws);
            if (verbose)
              message("telnet","SENT wont NAWS\n",hearer,this_object());
            break;
          default:
            my_socket_write(rec_fd,sprintf("%c%c%c",IAC,WONT,TELOPT_ECHO));
            if (verbose)
              message("telnet","SENT wont " + chunks[i][1] + "\n",hearer,
                      this_object());
            break;
        }
        if(strlen(chunks[i]) > 2)
          message("telnet",chunks[i][2..strlen(chunks[i])-1],hearer,
                  this_object());
        break;
      case WONT:
        if (verbose)
          message("telnet","RCVD wont "+telopts[chunks[i][1]]+"\n",hearer,
                  this_object());
        switch(chunks[i][1]){
          case TELOPT_ECHO:
            message("",s_iac_wont_echo,hearer,this_object());
            break;
          case TELOPT_SGA:
            message("",s_iac_wont_sga,hearer,this_object());
            break;
        }
        if(strlen(chunks[i]) > 2)
          message("telnet",chunks[i][2..strlen(chunks[i])-1],hearer,
                  this_object());
        break;
      case WILL:
        if (verbose)
          message("telnet","RCVD will "+telopts[chunks[i][1]]+"\n",hearer,
                  this_object());
        switch(chunks[i][1]){
          case TELOPT_ECHO:
            message("",s_iac_will_echo,hearer,this_object());
            my_socket_write(rec_fd,s_iac_do_echo);
            if (verbose)
              message("telnet","SENT do ECHO\n",hearer,this_object());
            break;
          case TELOPT_SGA:
            message("",s_iac_will_sga,hearer,this_object());
            break;
        }
        if(strlen(chunks[i]) > 2)
          message("telnet",chunks[i][2..strlen(chunks[i])-1],hearer,
                  this_object());
        break;
      default:
        message("telnet",chunks[i],hearer,this_object());
        break;
    }
  }
}

void write_data(int fd)
{
  write_state = WRITE_GO_AHEAD;
  my_socket_write(fd,"");
}

void socket_shutdown(int fd)
{
  object hearer;

  if(fd == conn_fd){
    hearer = environment(this_object());
    disconnected();
    return;
  }
}
