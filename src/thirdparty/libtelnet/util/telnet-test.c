/*
 * Sean Middleditch
 * sean@sourcemud.org
 *
 * The author or authors of this code dedicate any and all copyright interest
 * in this code to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and successors. We
 * intend this dedication to be an overt act of relinquishment in perpetuity of
 * all present and future rights to this code under copyright law. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "libtelnet.h"

typedef struct state {
	char *expected;
	char *actual;
} state_t;

static const telnet_telopt_t telopts[] = {
 { TELNET_TELOPT_COMPRESS2,	TELNET_WILL, TELNET_DONT },
 { TELNET_TELOPT_ZMP,		TELNET_WILL, TELNET_DONT },
 { TELNET_TELOPT_MSSP,		TELNET_WILL, TELNET_DONT },
 { TELNET_TELOPT_NEW_ENVIRON, TELNET_WILL, TELNET_DONT },
 { TELNET_TELOPT_TTYPE,		TELNET_WILL, TELNET_DONT },
 { -1, 0, 0 }
};

static const char *get_cmd(unsigned char cmd) {
	static char buffer[4];

	switch (cmd) {
	case 255: return "IAC";
	case 254: return "DONT";
	case 253: return "DO";
	case 252: return "WONT";
	case 251: return "WILL";
	case 250: return "SB";
	case 249: return "GA";
	case 248: return "EL";
	case 247: return "EC";
	case 246: return "AYT";
	case 245: return "AO";
	case 244: return "IP";
	case 243: return "BREAK";
	case 242: return "DM";
	case 241: return "NOP";
	case 240: return "SE";
	case 239: return "EOR";
	case 238: return "ABORT";
	case 237: return "SUSP";
	case 236: return "xEOF";
	default:
		snprintf(buffer, sizeof(buffer), "%d", (int)cmd);
		return buffer;
	}
}

static const char *get_opt(unsigned char opt) {
	switch (opt) {
	case 0: return "BINARY";
	case 1: return "ECHO";
	case 2: return "RCP";
	case 3: return "SGA";
	case 4: return "NAMS";
	case 5: return "STATUS";
	case 6: return "TM";
	case 7: return "RCTE";
	case 8: return "NAOL";
	case 9: return "NAOP";
	case 10: return "NAOCRD";
	case 11: return "NAOHTS";
	case 12: return "NAOHTD";
	case 13: return "NAOFFD";
	case 14: return "NAOVTS";
	case 15: return "NAOVTD";
	case 16: return "NAOLFD";
	case 17: return "XASCII";
	case 18: return "LOGOUT";
	case 19: return "BM";
	case 20: return "DET";
	case 21: return "SUPDUP";
	case 22: return "SUPDUPOUTPUT";
	case 23: return "SNDLOC";
	case 24: return "TTYPE";
	case 25: return "EOR";
	case 26: return "TUID";
	case 27: return "OUTMRK";
	case 28: return "TTYLOC";
	case 29: return "3270REGIME";
	case 30: return "X3PAD";
	case 31: return "NAWS";
	case 32: return "TSPEED";
	case 33: return "LFLOW";
	case 34: return "LINEMODE";
	case 35: return "XDISPLOC";
	case 36: return "ENVIRON";
	case 37: return "AUTHENTICATION";
	case 38: return "ENCRYPT";
	case 39: return "NEW-ENVIRON";
	case 70: return "MSSP";
	case 85: return "COMPRESS";
	case 86: return "COMPRESS2";
	case 93: return "ZMP";
	case 255: return "EXOPL";
	default: return "unknown";
	}
}

static char *append(char *original, const char *append) {
	size_t olen, alen;
	char *dest;
	
	olen = original != NULL ? strlen(original) : 0;
	alen = strlen(append);

	dest = (char *)malloc(olen + alen + 1);
	dest[0] = '\0';

	if (original != NULL) {
		strcpy(dest, original);

		free(original);
	}

	strcat(dest, append);
	return dest;
}

static void stprintf(state_t *state, char *format, ...) {
	char buffer[4096];
	va_list va;

	va_start(va, format);
	vsnprintf(buffer, sizeof(buffer), format, va);
	state->actual = append(state->actual, buffer);
	va_end(va);
}

static void decode(char *buffer, size_t *size) {
	const char *in = buffer, *end = buffer + *size;
	char *out = buffer;
	int c;

	while (in != end) {
		if (*in == '%') {
			++in;
			if (in == end) {
				break;
			}
			if (*in == '%') {
				*out = '%';
			} else {
				if (isdigit(*in)) {
					c = *in - '0';
				} else {
					c = *in - 'A' + 10;
				}
				++in;
				if (in == end) {
					break;
				}
				c *= 16;
				if (isdigit(*in)) {
					c += *in - '0';
				} else {
					c += *in - 'A' + 10;
				}
				*out = (char)c;
			}
			++out;
		} else if (isprint(*in)) {
			*out = *in;
			++out;
		}

		++in;
	}

	*out = '\0';
	*size = out - buffer;
}

static void print_encode(state_t *state, const char *buffer, size_t size) {
	const char *in = buffer, *end = buffer + size;

	while (in != end) {
		if (*in == '%') {
			stprintf(state, "%%%%");
		} else if (isprint(*in)) {
			stprintf(state, "%c", *in);
		} else {
			stprintf(state, "%%%02X", *in);
		}
		++in;
	}
}

static void event_print(telnet_t *telnet, telnet_event_t *ev, void *ud) {
	size_t i;
	state_t *state;

	(void)telnet;

	state = (state_t *)ud;

	switch (ev->type) {
	case TELNET_EV_DATA:
		stprintf(state, "DATA [%zi] ==> ", ev->data.size);
		print_encode(state, ev->data.buffer, ev->data.size);
		stprintf(state, "\n");
		break;
	case TELNET_EV_SEND:
		break;
	case TELNET_EV_IAC:
		stprintf(state, "IAC %d (%s)\n", (int)ev->iac.cmd, get_cmd(ev->iac.cmd));
		break;
	case TELNET_EV_WILL:
		stprintf(state, "WILL %d (%s)\n", (int)ev->neg.telopt, get_opt(ev->neg.telopt));
		break;
	case TELNET_EV_WONT:
		stprintf(state, "WONT %d (%s)\n", (int)ev->neg.telopt, get_opt(ev->neg.telopt));
		break;
	case TELNET_EV_DO:
		stprintf(state, "DO %d (%s)\n", (int)ev->neg.telopt, get_opt(ev->neg.telopt));
		break;
	case TELNET_EV_DONT:
		stprintf(state, "DONT %d (%s)\n", (int)ev->neg.telopt, get_opt(ev->neg.telopt));
		break;
	case TELNET_EV_SUBNEGOTIATION:
		switch (ev->sub.telopt) {
		case TELNET_TELOPT_ENVIRON:
		case TELNET_TELOPT_NEW_ENVIRON:
		case TELNET_TELOPT_TTYPE:
		case TELNET_TELOPT_ZMP:
		case TELNET_TELOPT_MSSP:
			/* print nothing */
			break;
		default:
			stprintf(state, "SUB %d (%s) [%zi]\n", (int)ev->sub.telopt, get_opt(ev->sub.telopt), ev->sub.size);
			break;
		}
		break;
	case TELNET_EV_ZMP:
		stprintf(state, "ZMP (%s) [%zi]\n", ev->zmp.argv[0], ev->zmp.argc);
		break;
	case TELNET_EV_TTYPE:
		stprintf(state, "TTYPE %s %s\n", ev->ttype.cmd ? "SEND" : "IS",
				ev->ttype.name ? ev->ttype.name : "");
		break;
	/* ENVIRON/NEW-ENVIRON commands */
	case TELNET_EV_ENVIRON:
		stprintf(state, "ENVIRON [%zi parts] ==> %s", ev->environ.size, ev->environ.cmd == TELNET_ENVIRON_IS ? "IS" : (ev->environ.cmd == TELNET_ENVIRON_SEND ? "SEND" : "INFO"));
		for (i = 0; i != ev->environ.size; ++i) {
			stprintf(state, " %s \"", ev->environ.values[i].type == TELNET_ENVIRON_VAR ? "VAR" : "USERVAR");
			if (ev->environ.values[i].var != 0) {
				print_encode(state, ev->environ.values[i].var, strlen(ev->environ.values[i].var));
			}
			stprintf(state, "\"");
			if (ev->environ.cmd != TELNET_ENVIRON_SEND) {
				stprintf(state, "=\"");
				if (ev->environ.values[i].value != 0) {
					print_encode(state, ev->environ.values[i].value, strlen(ev->environ.values[i].value));
				}
				stprintf(state, "\"");
			}
		}
		stprintf(state, "\n");
		break;
	case TELNET_EV_MSSP:
		stprintf(state, "MSSP [%zi] ==>", ev->mssp.size);
		for (i = 0; i != ev->mssp.size; ++i) {
			stprintf(state, " \"");
			print_encode(state, ev->mssp.values[i].var, strlen(ev->mssp.values[i].var));
			stprintf(state, "\"=\"");
			print_encode(state, ev->mssp.values[i].value, strlen(ev->mssp.values[i].value));
			stprintf(state, "\"");
		}
		stprintf(state, "\n");
		break;
	case TELNET_EV_COMPRESS:
		stprintf(state, "COMPRESSION %s\n", ev->compress.state ? "ON" : "OFF");
		break;
	case TELNET_EV_WARNING:
		stprintf(state, "WARNING: %s\n", ev->error.msg);
		break;
	case TELNET_EV_ERROR:
		stprintf(state, "ERROR: %s\n", ev->error.msg);
		break;
	}
}

int main(int argc, char** argv) {
	FILE *fh;
	telnet_t *telnet;
	char buffer[4096];
	size_t len;
	state_t state;

	state.expected = NULL;
	state.actual = NULL;

	/* check for a requested input file */
	if (argc != 3) {
		fprintf(stderr, "Usage: telnet-test [test file] [expected outline]\n");
		return 1;
	}

	/* load expected input file */
	if ((fh = fopen(argv[2], "rt")) == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n",
				argv[2], strerror(errno));
		return 2;
	}

	while (fgets(buffer, sizeof(buffer), fh) != NULL) {
		state.expected = append(state.expected, buffer);
	}

	fclose(fh);

	/* open input file file */
	if ((fh = fopen(argv[1], "rt")) == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n",
				argv[1], strerror(errno));
		return 3;
	}

	/* create telnet parser instance */
	if ((telnet = telnet_init(telopts, event_print, 0,
			&state)) == 0) {
		fprintf(stderr, "Failed to initialize libtelnet: %s\n",
				strerror(errno));
		fclose(fh);
		return 4;
	}

	/* read input until we hit EOF or marker */
	while (fgets(buffer, sizeof(buffer), fh) != NULL && strcmp(buffer, "%%\n") != 0) {
		if (buffer[0] != '#') {
			len = strlen(buffer);
			decode(buffer, &len);
			telnet_recv(telnet, buffer, len);
		}
	}

	fclose(fh);

	/* compare expected and actual output */
	if (strcmp(state.actual, state.expected) != 0) {
		fprintf(stderr, "Expected:\n%s\n\nActual:\n%s\n", state.expected, state.actual);
		return 5;
	}

	/* clean up */
	telnet_free(telnet);
	free(state.expected);
	free(state.actual);

	return 0;
}
