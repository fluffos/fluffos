#ifndef LEXER_UTILS_H
#define LEXER_UTILS_H

#include <unordered_map>
#include <string>
#include <string_view>
#include <utility>

struct PredefMacro {
    bool is_function_like;
    int nargs;
    std::string body;
};

// Predefines
void add_predefine(std::string_view name, int nargs, std::string_view exps);
void add_quoted_predefine(std::string_view def, std::string_view val);
void add_predefines();
const std::unordered_map<std::string, PredefMacro>& get_predefines();
// Bumped on every add_predefine/add_quoted_predefine: lets the pp layer
// cache a PpMacro-shaped view of the registry and rebuild only on change.
unsigned get_predefines_version();

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_all_defines();
#endif

void set_inc_list(const char *list);
void init_include_path();
void deinit_include_path();
std::pair<int, std::string> inc_open(std::string_view name, bool check_local);


#endif
