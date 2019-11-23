/// @file shared_string.hh
/// @brief storage class for shared strings
/// @author René Müller
/// @version 0.0.1
/// @date 2019-07-21

#ifndef _SHARED_STRING_HH_
#define _SHARED_STRING_HH_

#include <string>
#include <fmt/format.h>

#include "templates/shared_storage.hh"

using shared_string = shared_storage<std::string>;

namespace fmt {
    // format shared_string as std::string
    template <>
        struct formatter<shared_string> : formatter<std::string> {
            template <typename FormatContext>
                std::string format(const shared_string &p, FormatContext &ctx) {
                    return formatter<std::string>::format(*p, ctx);
                }
        };
}

#endif // SHARED_STRING_HH
