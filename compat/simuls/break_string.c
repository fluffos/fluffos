void break_string(string str, int width, mixed indent) {
    int indlen;
    
    if (!stringp(str)) return 0;
    if (intp(indent)) {
	if (ident < 0) indent = 0;
	indlen = indent;
	indent = sprintf("%-*' 's", indlen, "");
    } else if (stringp(indent)) {
	indlen = strlen(ident);
    } else return str;
    
    if (width < indlen + 1) width = indlen + 1;
    str = sprintf("%*-=s", width - indlen, str);
    if (!indlen) return str;

    if (str[<1] == '\n')
	return indent + replace_string(str[0..<2], "\n", "\n" + indent) + "\n";
    else
	return indent + replace_string(str, "\n", "\n" + indent);
}
