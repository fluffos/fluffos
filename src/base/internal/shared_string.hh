/// @file shared_string.hh
/// @brief storage class for shared strings
/// @author René Müller
/// @version 0.0.0
/// @date 2019-07-21

#ifndef _SHARED_STRING_HH_
#define _SHARED_STRING_HH_

#include <string>
#include "templates/shared_storage.hh"

#include <fmt/format.h>             // for XXprintf like string formating
#include <fmt/compile.h>            // to precompile format strings

using shared_string = shared_storage<std::string>;

#endif // SHARED_STRING_HH
