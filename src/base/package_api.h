/*
 * package_api.h
 *
 * This is the single file for all package to include instead of std.h
 *
 */

#ifndef SRC_BASE_PACKAGE_API_H_
#define SRC_BASE_PACKAGE_API_H_

// FIXME: avoid this?
#include "base/std.h"

// details about machine
#include "vm/vm.h"

// FIXME: apply_cache
#include "vm/internal/base/apply_cache.h"

// FIXME: efuns_main.cc needs this
#include "vm/internal/otable.h"

// FIXME: sprintf needs query_instr_name
#include "vm/internal/compiler/lex.h"

// FIXME: disassembler needs icode
#include "vm/internal/compiler/icode.h"

// APIs to LPC programs
#include "include/localtime.h"
#ifdef PACKAGE_PARSER
#include "include/parser_error.h"
#endif
#ifdef PACKAGE_SOCKETS
#include "include/socket_err.h"
#endif

// FIXME: backend queue
#include "backend.h"
#include "event.h"
#include "comm.h"  // reverse API, FIXME
#include "user.h"  // reverse API

#endif /* SRC_BASE_PACKAGE_API_H_ */
