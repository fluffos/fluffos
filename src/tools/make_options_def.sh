#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

TOP_SRCDIR=${1}
shift
PROG=${1}
shift
TARGET=${1}
shift

ALL_OPTIONS_H=${TOP_SRCDIR}/src/base/internal/options_incl.h
"$@" -E -undef -dM $ALL_OPTIONS_H | python3 ${PROG} ${TARGET}