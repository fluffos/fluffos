#ifdef COMPAT_TAIL
/* This version is strictly compatible with the old version */
int tail(string fname) {
     string str;
     int offset = file_size(fname);

     if (offset < 0)
	 return 0;
     
     offset -= 54 * 20;
     if (offset < 0) offset = 0;
     str = read_bytes(fname, offset, 1080);
     if (!str) return 0;
     if (offset) str = str[strsrch(str, "\n")+1..];
     write(str);

     return 1;
}
#else
/* This version is slightly extended and compatible in spirit, but doesn't
 * reproduce the oddities of the original tail() efun.  Note that it also
 * returns the string, so write(tail(fname)) is needed for strict 
 * compatibility.
 */
varargs string tail(string fname, int nlines) {
    int chunk = nlines * 80;
    int offset = file_size(fname);
    int num_nl, p, skip;
    string str = "";

    while (offset > 0 && num_nl <= nlines) {
	num_nl = 0;
	offset -= chunk;
	if (offset < 0) {
	    chunk += offset; /* negative */
	    offset = 0;
	}
	str = read_bytes(fname, offset, chunk) + str;
	p = -1;
	while (p < sizeof(str)-1 && p = member_array('\n', str, p+1))
	    num_nl++;
    }
    skip = num_nl - nlines;
    p = -1;
    while (skip--)
	p = member_array('\n', str, p+1);
    return str[p..];
}
#endif
