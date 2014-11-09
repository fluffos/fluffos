/*
 * heartbeat.h
 *
 *  Created on: Nov 5, 2014
 *      Author: sunyc
 */

#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_

#include "std.h"
#include "lpc_incl.h"

// FIXME: remove this usage
extern object_t *g_current_heartbeat_obj;

int set_heart_beat(object_t *, int);
int query_heart_beat(object_t *);
int heart_beat_status(outbuffer_t *, int);
array_t *get_heart_beats();

// Used by md.cc for verifying.
void check_heartbeats();
// Shutdown hook
void clear_heartbeats();

#endif /* HEARTBEAT_H_ */
