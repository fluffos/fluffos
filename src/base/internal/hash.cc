#include "base/std.h"

#include "hash.h"

unsigned long whashstr(const std::string s) {
    size_t i = 0;

    unsigned long __h = 0;
    for (; i < s.size() && i < 100;) {
        __h = 37 * __h + s[i++];
    }
    return __h;
}
