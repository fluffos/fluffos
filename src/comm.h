#include <sys/socket.h>
#ifdef _AIX
#include <sys/socketvar.h>
#endif
#include <netinet/in.h>
#ifndef hpux
#include <arpa/inet.h>
#endif

#define MAX_TEXT	2048

#define MAX_SOCKET_PACKET_SIZE	1024	/* Wild guess. */
#define DESIRED_SOCKET_PACKET_SIZE 800

#define MESSAGE_FLUSH ((char*)NULL)

struct interactive {
    int socket;
    struct object *ob;		/* Points to the associated object */
    struct sentence *input_to;	/* To be called with next input line ! */
    struct sockaddr_in addr;
    char *prompt;
    int closing;		/* True when closing this socket. */
    int do_close;		/* This is to be closed down. */
    char text[MAX_TEXT];
    int text_end;		/* first free char in buffer */
    int text_start;		/* where we are up to in player cmd buffer */
    struct interactive *snoop_on, *snoop_by;
#ifdef PORTALS
    int from_portal;		/* True if this player has arrived through a portal */
    int portal_socket;		/* All messages should go through this socket, if open */
    int out_portal;		/* True if going out through a portal. */
#endif /* PORTALS */
    int noecho;			/* Don't echo lines */
    int noesc;                  /* Don't allow shell out */
    int last_time;		/* Time of last command executed */
    char *default_err_message;	/* This or What ? is printed when error */
#ifdef ACCESS_RESTRICTED
    void *access_class;         /* represents a "cluster" where this player comes from */                              
#endif
    int trace_level;		/* Debug flags. 0 means no debugging */
    char *trace_prefix;		/* Trace only object which has this as name prefix */
    struct ed_buffer *ed_buffer;	/* Local ed */
    int message_length;
    char message_buf[DESIRED_SOCKET_PACKET_SIZE+1];
    struct object *next_player_for_flush, *previous_player_for_flush;
#ifdef MUDWHO
    int login_time;
#endif
    int has_process_input;
    int has_write_prompt;
    int single_char; /* single character mode */
};

/* input_to flags */
#define I_NOECHO	0x1
#define	I_NOESC		0x2
