/**
 * @file shared_string.cc
 * @brief utility functions for LPC-strings
 * @author René Müller
 * @version 0.0.0
 * @date 2019-12-30
 */

#include "base/internal/shared_string.hh"

shared_string::status {0,0,USHRT_MAX,0};

/**
 * @brief adds status informations about shared strings to given outbuffer
 *
 * @param out           outbuffer to use
 * @param verbose       flag for brief/verbose status
 *
 * @return  used storage + overhead
 */
int add_string_status(outbuffer_t *out, int verbose) {
    shared_string::status_t status {shared_string::get_status(verbose)};
    size_t ret {status.size * sizeof(shared_string::value_t)};

    if (verbose) {
        outbuf_add(out, "Shared strings:\n-------------------------\t Strings    Size       Overhead\n");
        outbuf_addv(out, "All                      \t {: 8}   {: 8}   {: 8}\n",
                status.size, status.data_size, status.size * sizeof(shared_string::value_t));
        outbuf_addv(out, "Unreferenced             \t {: 8}   {: 8}   {: 8}\n",
                status.unref, status.unref_size, status.unref_size * sizeof(shared_string::value_t));
        outbuf_addv(out, "Protected                \t {: 8}   {: 8}   {: 8}\n\n",
                status.prot, status.prot_size, status.prot_size * sizeof(shared_string::value_t));
        outbuf_add(out, "Shared strings container:\n-------------------------\n");
        outbuf_addv(out, "Buckets available        \t {: 8}\n", status.max_buckets);
        outbuf_addv(out, "Buckets used             \t {: 8}\n", status.num_buckets);
        outbuf_addv(out, "Max bucket size avail.   \t {: 8}\n", status.max_bucket_size);
        outbuf_addv(out, "Average bucket size used \t {: 8}\n", status.av_bucket_size);
    }
    else {
        outbuf_addv(out, "Strings all/unref/prot   \t {: 8}   {: 8}   {: 8}\n",
                status.size, status.unrefm status.prot);
        outbuf_addv(out, "Buckets avail/av. size   \t {: 8}   {: 8}\n",
                status.num_buckets, status.av_bucket_size);
    }
    return ret;

#if 0       // from stralloc.cc::add_srting_status(...)
    if (verbose == 1) {
        outbuf_add(out, "All strings:\n");
        outbuf_add(out, "-------------------------\t Strings    Bytes\n");
    }
    if (verbose != -1) {
        outbuf_addv(out, "All strings:\t\t\t%8d %8d + %d overhead\n", num_distinct_strings,
        bytes_distinct_strings, overhead_bytes);
    }
    if (verbose == 1) {
        outbuf_addv(out, "Total asked for\t\t\t%8d %8d\n", allocd_strings, allocd_bytes);
        outbuf_addv(out, "Space actually required/total string bytes %d%%\n",
        (bytes_distinct_strings + overhead_bytes) * 100 / allocd_bytes);
        outbuf_addv(out, "Searches: %d    Average search length: %6.3f\n", num_str_searches,
        static_cast<double>(search_len) / num_str_searches);
    }
    return (bytes_distinct_strings + overhead_bytes);
#endif
}
