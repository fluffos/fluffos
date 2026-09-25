---
title: pcre / pcre_match_all
---
# pcre_match_all

### NAME

    pcre_match_all() - find all matches

### SYNOPSIS

    mixed pcre_match_all(string input, string pattern, void|int|mapping pcre_flags);

### DESCRIPTION

    Similiar to php preg_match_all, this EFUN returns a array of string arrays,
    containing all matches and captured groups.

### Example

    // https://tools.ietf.org/html/rfc3986#appendix-B
    pcre_match_all("http://www.ics.uci.edu/pub/ietf/uri/#Related",
                       "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?" , PCRE_M));

    Will return
      ({ /* sizeof() == 1 */
        ({ /* sizeof() == 10 */
          "http://www.ics.uci.edu/pub/ietf/uri/#Related",
              "http:",
              "http",
              "//www.ics.uci.edu",
              "www.ics.uci.edu",
              "/pub/ietf/uri/",
              "",
              "",
              "#Related",
              "Related"
        })
      }),

    There are 1 match in the entire string, the first item in the array is the
    matched substring, then all the captured groups.

:::warning[Unicode properties are on by default]

Patterns compile with `PCRE2_UCP`, so `\d` matches any Unicode digit and `\w`
any letter. Patterns used to *validate* input now accept more than they did
before the PCRE2 upgrade. Pass `PCRE_NO_UCP` (or `([ "no_ucp": 1 ])`) to get the
ASCII-only classes back -- see [pcre_match](pcre_match) for the full note.

:::
