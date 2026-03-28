
string pcre_version(void);
mixed pcre_match(string | string *, string, void | int, void | int);
mixed pcre_match_all(string, string, void | int);
mixed *pcre_assoc(string, string *, mixed *, mixed | void, void | int);
string *pcre_extract(string, string, void | int, void | int);
string pcre_replace(string, string, string *, void | int);
string pcre_replace_callback(string, string, string | function, ..., void | int);
mapping pcre_cache(void);
