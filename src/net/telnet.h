#ifndef TELNET_H_
#define TELNET_H_

// Because libtelnet already uses telnet_init.
struct telnet_t *net_telnet_init(struct interactive_t *user);

// Send initial negotiations.
void send_initial_telnet_negotiations(struct interactive_t *user);

// Change terminal mode
void set_linemode(struct interactive_t *ip, bool flush = true);
void set_charmode(struct interactive_t *ip, bool flush = true);
void set_localecho(struct interactive_t *ip, bool enable, bool flush = true);

void telnet_do_naws(struct telnet_t *);
void telnet_dont_naws(struct telnet_t *);
void telnet_start_request_ttype(struct telnet_t *);
void telnet_request_ttype(struct telnet_t *);
void telnet_request_term_size(struct telnet_t *);

#endif /* TELNET_H_ */
