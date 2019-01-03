#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

TOP_SRCDIR=${1}
shift

ALL_OPTIONS_H=${TOP_SRCDIR}/src/base/internal/options_incl.h

PACKAGES_SPEC=packages.spec
PACKAGES_SPEC_FULL=packages.fullspec

# do pre-cleanup
rm -f "$PACKAGES_SPEC" "PACKAGES_SPEC_FULL" >/dev/null 2>&1 || true

PACKAGES=$("$@" -E -undef -dM $ALL_OPTIONS_H | grep PACKAGE_ | cut -d " " -f 2)

for P in $PACKAGES; do
	# get the package name, in uppercase
	P=${P#PACKAGE_} 
	# get the directory and file name
	DIR=$(echo $P | tr '[:upper:]' '[:lower:]')

	if [[ -e "${TOP_SRCDIR}/src/packages/$DIR/$DIR.spec" ]]; then
		echo "#include \"${TOP_SRCDIR}/src/packages/$DIR/$DIR.spec\"" >> $PACKAGES_SPEC
	else
		echo "Warning: ${TOP_SRCDIR}/src/packages/$DIR/$DIR.spec is missing."
	fi
done

"$@" -E -imacros $ALL_OPTIONS_H -P -x c++ $PACKAGES_SPEC > $PACKAGES_SPEC_FULL
