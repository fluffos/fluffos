#!/bin/bash
set -euo pipefail
IFS=$'\n\t'

ALL_OPTIONS_H=base/internal/options_incl.h

PACKAGES_SPEC=packages/packages.autogen.spec
PACKAGES_SPEC_FULL=packages/packages.autogen.fullspec
PACKAGES_MAKEFILE=packages/packages.autogen.makefile

# do cleanup
rm $PACKAGES_SPEC 2>&1 >/dev/null || true
rm $PACKAGES_MAKEFILE 2>&1 >/dev/null || true
rm $PACKAGES_SPEC_FULL 2>&1 >/dev/null || true

PACKAGES=$("$@" -E -undef -dM $ALL_OPTIONS_H | grep PACKAGE_ | cut -d " " -f 2)

for P in $PACKAGES; do
	# get the package name, in uppercase
	P=${P#PACKAGE_} 
	# get the directory and file name
	DIR=$(echo $P | tr '[:upper:]' '[:lower:]')
	  
	if [[ -e "packages/$DIR/$DIR.spec" ]]; then
		echo "#include \"packages/$DIR/$DIR.spec\"" >> $PACKAGES_SPEC;
	else
		echo "Warning: packages/$DIR/$DIR.spec is missing."
	fi

	# Build makefile rules
	if [[ -e "packages/$DIR/$DIR.makefile" ]]; then
		echo "include packages/$DIR/$DIR.makefile" >> $PACKAGES_MAKEFILE
	fi
	echo "PACKAGE_${P}_OBJS= \\" >> $PACKAGES_MAKEFILE
	for F in packages/$DIR/*.cc; do
		echo -e "\t${F%.cc}.o \\" >> $PACKAGES_MAKEFILE
	done

	echo " " >> $PACKAGES_MAKEFILE
done

echo " " >> $PACKAGES_MAKEFILE
echo " " >> $PACKAGES_MAKEFILE
echo "PACKAGES_ALL_OBJS= \\" >> $PACKAGES_MAKEFILE
for P in $PACKAGES; do
	P=${P#PACKAGE_}
	echo -e "\t\$(PACKAGE_${P}_OBJS) \\" >> $PACKAGES_MAKEFILE
done

echo " " >> $PACKAGES_MAKEFILE

# Build fully expanded efun specs
"$@" -E -imacros $ALL_OPTIONS_H -P -x c++ $PACKAGES_SPEC > $PACKAGES_SPEC_FULL