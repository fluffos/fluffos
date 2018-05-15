/*
 * stats.h
 *
 * This files contians all statistic collected by various part of the driver.
 *
 * TODO: encapsulate into an class.
 */

#ifndef STATS_H
#define STATS_H

#include <inttypes.h>

#include "base/internal/options_incl.h"

extern uint64_t add_message_calls;

extern uint64_t inet_packets;
extern uint64_t inet_volume;

extern uint64_t inet_out_packets;
extern uint64_t inet_out_volume;
extern uint64_t inet_in_packets;
extern uint64_t inet_in_volume;

extern uint64_t inet_socket_in_packets;
extern uint64_t inet_socket_in_volume;
extern uint64_t inet_socket_out_packets;
extern uint64_t inet_socket_out_volume;

// Compiler stats
extern uint64_t total_num_prog_blocks, total_prog_block_size;

// Object stats
extern uint64_t tot_alloc_object, tot_alloc_object_size;
extern uint64_t tot_dangling_object;

// Array stats
extern uint64_t num_arrays, total_array_size;

// Class stats
extern uint64_t num_classes, total_class_size;

// Mapping stats
extern uint64_t num_mappings, total_mapping_size, total_mapping_nodes;

// Apply cache stats
extern uint64_t apply_cache_lookups;
extern uint64_t apply_cache_hits;
extern uint64_t apply_cache_items;

#endif
