#!/bin/sh

echo Converting man pages...
find . -name "*.[34]" -exec convert.sh {} \;
echo Done.
