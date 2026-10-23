#ifndef PACKAGES_CORE_REPLACE_STRING_H
#define PACKAGES_CORE_REPLACE_STRING_H

namespace vm {
class Value;
class StringArg;
}  // namespace vm

/* replace_string()'s body (efuns_main.cc), shared with dwlib's replace().
 * It owns `source`: a string nothing else references is rewritten in place
 * when the replacement is not longer than the pattern. `first`/`last` are
 * the efun's range arguments; last == 0 means up to the maximum string
 * length. */
vm::Value replace_string_value(vm::Value source, vm::StringArg pattern, vm::StringArg replacement,
                               LPC_INT first = 0, LPC_INT last = 0);

#endif /* PACKAGES_CORE_REPLACE_STRING_H */
