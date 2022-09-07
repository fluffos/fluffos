#pragma once

void on_telnet_do_msp(struct interactive_t *);
void on_telnet_dont_msp(struct interactive_t *);

void telnet_send_msp_oob(interactive_t *, const char *, size_t);
