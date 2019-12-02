/// @file shared_string.hh
/// @brief storage class for shared strings
/// @author René Müller
/// @version 0.0.1
/// @date 2019-07-21

#ifndef _SHARED_STRING_HH_
#define _SHARED_STRING_HH_

#include <string>
#include <memory>
#include <fmt/format.h>

#include "templates/shared_storage.hh"

using shared_string         = shared_storage<std::string>;
using shared_string_ptr_t   = std::unique_ptr<shared_string>;

namespace fmt {
    // format shared_string as std::string
    template <>
        struct formatter<shared_string> : formatter<string_view> {
            template <typename FormatContext>
                auto format(const shared_string &p, FormatContext &ctx) {
                    return formatter<string_view>::format(*p, ctx);
                }
        };
}

#endif // SHARED_STRING_HH
