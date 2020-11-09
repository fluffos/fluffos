
string pcre_version(void);
mixed pcre_match(string | string *, string, void | int);
mixed pcre_match_all(string, string);
mixed *pcre_assoc(string, string *, mixed *, mixed | void);
string *pcre_extract(string, string);
string pcre_replace(string, string, string *);
string pcre_replace_callback(string, string, string | function, ...);
mapping pcre_cache(void);
