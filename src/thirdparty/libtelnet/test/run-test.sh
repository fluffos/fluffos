DIR=$(dirname "$0")
RS=0

while [ "x$1" != "x" ] ; do
	echo -ne "TEST $1\t\t\t"

	RUNTMP="$(basename "$1").run.tmp"
	OUTTMP="$(basename "$1").out.tmp"

	"../util/telnet-test" "$1" > "$RUNTMP"
	sed -n '/%%/,$p' < "$1" | tail -n+2 > "$OUTTMP"
	if cmp -s "$OUTTMP" "$RUNTMP" ; then
		echo "OK"
	else
		echo "FAIL"
		echo "EXPECTED:"
		sed 's/^/\t/' < "$OUTTMP"
		echo "GOT:"
		sed 's/^/\t/' < "$RUNTMP"
		RS=1
	fi
	rm -f "$RUNTMP" "$OUTTMP"

	shift
done

exit $RS
