#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

TOP_SRCDIR=${1}
shift

ALL_OPTIONS_H=${TOP_SRCDIR}/src/base/internal/options_incl.h

PACKAGES_SPEC=packages.spec
PACKAGES_SPEC_FULL=packages.fullspec
PACKAGES_MAKEFILE=packages.makefile
TMP_FILE=`mktemp -p . autogen.XXXXXXXXXX`

# do pre-cleanup
# rm -f "$PACKAGES_SPEC" "$PACKAGES_MAKEFILE" "$PACKAGES_SPEC_FULL" >/dev/null 2>&1 || true
rm -f "$PACKAGES_SPEC" "$PACKAGES_MAKEFILE" >/dev/null 2>&1 || true

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

### 	# Build makefile rules
### 	if [[ -e "${TOP_SRCDIR}/src/packages/$DIR/$DIR.makefile" ]]; then
### 		echo "include ${TOP_SRCDIR}/src/packages/$DIR/$DIR.makefile" >> $PACKAGES_MAKEFILE
### 	fi
### 	echo "PACKAGE_${P}_OBJS= \\" >> $PACKAGES_MAKEFILE
### 	for F in ${TOP_SRCDIR}/src/packages/$DIR/*.cc; do
### 		echo -e "\t${F%.cc}.o \\" >> $PACKAGES_MAKEFILE
### 	done

### 	echo " " >> $PACKAGES_MAKEFILE
done

### echo " " >> $PACKAGES_MAKEFILE
### echo " " >> $PACKAGES_MAKEFILE
### echo "PACKAGES_ALL_OBJS= \\" >> $PACKAGES_MAKEFILE
### for P in $PACKAGES; do
### 	P=${P#PACKAGE_}
### 	echo -e "\t\$(PACKAGE_${P}_OBJS) \\" >> $PACKAGES_MAKEFILE
### done

### echo " " >> $PACKAGES_MAKEFILE

# Build fully expanded efun specs
# "$@" -E -imacros $ALL_OPTIONS_H -P -x c++ $PACKAGES_SPEC > $PACKAGES_SPEC_FULL
"$@" -E -imacros $ALL_OPTIONS_H -P -x c++ $PACKAGES_SPEC > $TMP_FILE
rm -f "$PACKAGES_SPEC" >/dev/null 2>&1 || true
if [[ -f $PACKAGES_SPEC_FULL ]]
then
    if diff "$TMP_FILE" "$PACKAGES_SPEC_FULL" > /dev/null 2>&1
    then
        echo Keeping old version
        rm -f "$TMP_FILE" >/dev/null 2>&1 || true
    else
        echo Installing changed version
        mv "$TMP_FILE" "$PACKAGES_SPEC_FULL"
    fi
else
    echo Installing new version
    mv "$TMP_FILE" "$PACKAGES_SPEC_FULL"
fi
true
