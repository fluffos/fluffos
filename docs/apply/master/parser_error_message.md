---
layout: doc
title: master / parser_error_message
---
# parser_error_message

### NAME

    parser_error_message - generate custom parser error messages

### SYNOPSIS

    string parser_error_message(int error_type, object ob, ...) ;

### DESCRIPTION

    This apply is called by the parser package when a parsing error occurs,
    allowing the mudlib to provide custom error messages for parse_sentence()
    failures.

    **Arguments:**
    - `error_type`: The type of parsing error that occurred
    - `ob`: The object that caused the error (may be 0)
    - Additional arguments depend on the error type

    **Error Types and Arguments:**

    - `ERR_IS_NOT` / `ERR_NOT_LIVING` / `ERR_NOT_ACCESSIBLE` (4 arguments):
      - `string noun`: The noun that caused the error
      - `int is_plural`: 1 if plural, 0 if singular

    - `ERR_AMBIG` (3 arguments):
      - `object *objects`: Array of ambiguous objects

    - `ERR_ORDINAL` (3 arguments):
      - `int ordinal`: The problematic ordinal number

    - `ERR_THERE_IS_NO` (3 arguments):
      - `string phrase`: The problematic word sequence

    - `ERR_ALLOCATED` (3 arguments):
      - `string message`: An allocated error string

    - `ERR_BAD_MULTIPLE` (2 arguments):
      - (no additional arguments)

    - `ERR_MANY_PATHS` (5 arguments):
      - (additional path-related data)

    **Return Value:**
    - Return a string containing the error message to show to the user
    - Return 0 to use default error handling

### EXAMPLE

    ```c
    string parser_error_message(int type, object ob, mixed arg) {
        switch(type) {
            case ERR_NOT_LIVING:
                return "The " + arg + " is not alive.";
            case ERR_AMBIG:
                return "Which " + file_name(arg[0]) + " do you mean?";
            case ERR_THERE_IS_NO:
                return "I don't see any '" + arg + "' here.";
            default:
                return "I don't understand that.";
        }
    }
    ```

### SEE ALSO

    parse_sentence(3), parse_init(3)

### NOTE

    This apply requires the parser package to be enabled. Error type constants
    should be defined in your mudlib's include files based on the parser
    package implementation.
