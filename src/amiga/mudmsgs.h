/* hosts/amiga/mudmsgs.h
**
** Define the structure of the messages the socket-simulation uses
** for communication.
*/

struct connect_message {
  struct Message Msg;
  char *port_name;
};

struct data_message {
  struct Message Msg;
  char *buffer;
  long length;
};



