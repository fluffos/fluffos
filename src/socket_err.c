/*
 * socket_errors.c -- socket error strings
 *    5-92 : Dwayne Fontenot (Jacques@TMI) : original coding.
 *   10-92 : Dave Richards (Cynosure) : less original coding.
 */

#include "include/socket_err.h"

const char *error_strings[ERROR_STRINGS] =
{
    "Problem creating socket",
    "Problem with setsockopt",
    "Problem setting non-blocking mode",
    "No more available efun sockets",
    "Descriptor out of range",
    "Socket is closed",
    "Security violation attempted",
    "Socket is already bound",
    "Address already in use",
    "Problem with bind",
    "Problem with getsockname",
    "Socket mode not supported",
    "Socket not bound to an address",
    "Socket is already connected",
    "Problem with listen",
    "Socket not listening",
    "Operation would block",
    "Interrupted system call",
    "Problem with accept",
    "Socket is listening",
    "Problem with address format",
    "Operation already in progress",
    "Connection refused",
    "Problem with connect",
    "Socket not connected",
    "Object type not supported",
    "Problem with sendto",
    "Problem with send",
    "Wait for callback",
    "Socket already released",
    "Socket not released",
    "Data nested too deeply"
};
