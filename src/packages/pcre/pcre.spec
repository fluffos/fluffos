
string pcre_version(void);
mapping pcre_config(void);
mapping pcre_info(string, void | int | mapping);
string pcre_convert(string, mapping);
mixed pcre_match(string | string *, string, void | int | mapping, void | int | mapping);
mixed pcre_match_all(string, string, void | int | mapping);
mixed *pcre_assoc(string, string *, mixed *, mixed | void, void | int | mapping);
string *pcre_extract(string, string, void | int | mapping, void | int | mapping);
string pcre_replace(string, string, string * | string, void | int | mapping);
/* The callback's extra arguments may be followed by the usual options
 * (int | mapping), recognised by type at the end of the tail. */
string pcre_replace_callback(string, string, string | function, ...);
mapping pcre_cache(void);
