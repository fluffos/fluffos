#include "spec.h"

string pcre_version(void);
mixed pcre_match(string | string *, string, void | int);
mixed *pcre_assoc(string, string *, mixed *, mixed | void);
string *pcre_extract(string, string);
string pcre_replace(string, string, string *);
string pcre_replace_callback(string, string, string | function, ...);
mapping pcre_cache(void);
