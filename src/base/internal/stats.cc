/*
 * stats.cc
 */

#include "base/internal/stats.h"

/*
 * public local variables.
 */
uint64_t add_message_calls = 0;

uint64_t inet_packets = 0;
uint64_t inet_volume = 0;

uint64_t inet_out_packets = 0;
uint64_t inet_out_volume = 0;
uint64_t inet_in_packets = 0;
uint64_t inet_in_volume = 0;
uint64_t inet_socket_in_packets = 0;
uint64_t inet_socket_in_volume = 0;
uint64_t inet_socket_out_packets = 0;
uint64_t inet_socket_out_volume = 0;

// Compiler stats
uint64_t total_num_prog_blocks = 0;
uint64_t total_prog_block_size = 0;

// Object stats
uint64_t tot_alloc_object = 0;
uint64_t tot_alloc_object_size = 0;
uint64_t tot_dangling_object = 0;

// Array stats
uint64_t num_arrays = 0;
uint64_t total_array_size = 0;

// Class stats
uint64_t num_classes = 0;
uint64_t total_class_size = 0;

// Mapping stats
uint64_t num_mappings = 0;
uint64_t total_mapping_size = 0;
uint64_t total_mapping_nodes = 0;

// Apply cache stats
uint64_t apply_cache_lookups = 0;
uint64_t apply_cache_hits = 0;
uint64_t apply_cache_items = 0;
