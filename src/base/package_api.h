/*
 * package_api.h
 *
 * This is the single file for all package to include instead of std.h
 *
 */

#ifndef SRC_BASE_PACKAGE_API_H_
#define SRC_BASE_PACKAGE_API_H_
// IWYU pragma: begin_exports

// FIXME: avoid this?
#include "base/std.h"

// details about machine
#include "vm/vm.h"

#include "interactive.h"

// FIXME: apply_cache
#include "vm/internal/base/apply_cache.h"

// FIXME: efuns_main.cc needs this
#include "vm/internal/otable.h"

// FIXME: sprintf needs query_instr_name
#include "compiler/internal/lex.h"

// FIXME: disassembler needs icode
#include "compiler/internal/icode.h"

// APIs to LPC programs
#include "compiler/internal/disassembler.h"

#include "include/localtime.h"
#ifdef PACKAGE_PARSER
#include "include/parser_error.h"
#endif
#ifdef PACKAGE_SOCKETS
#include "include/socket_err.h"
#endif

// FIXME: backend queue
#include "backend.h"
#include "comm.h"  // reverse API, FIXME
#include "user.h"  // reverse API

#include "net/telnet.h"  // Telnet related stuff

// trace.cc
extern struct array_t *get_svalue_trace();

// Tracing support
#include "base/internal/tracing.h"

// IWYU pragma: end_exports
#endif /* SRC_BASE_PACKAGE_API_H_ */
