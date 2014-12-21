/*
 * heartbeat.h
 *
 *  Created on: Nov 5, 2014
 *      Author: sunyc
 */

#ifndef HEARTBEAT_H_
#define HEARTBEAT_H_

// FIXME: remove this usage
extern struct object_t *g_current_heartbeat_obj;

int set_heart_beat(struct object_t *, int);
int query_heart_beat(struct object_t *);
int heart_beat_status(struct outbuffer_t *, int);
struct array_t *get_heart_beats();

// Used by backend.cc
void call_heart_beat();

// Used by md.cc for verifying.
void check_heartbeats();
// Shutdown hook
void clear_heartbeats();

#endif /* HEARTBEAT_H_ */
