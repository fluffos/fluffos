---
layout: default
title: pcre / pcre_match_all
---

### NAME

    pcre_match_all() - find all matches

### SYNOPSIS

    mixed pcre_match_all(string, string);

### DESCRIPTION

    Similiar to php preg_match_all, this EFUN returns a array of string arrays,
    containing all matches and captured groups.

### Example
    // https://tools.ietf.org/html/rfc3986#appendix-B
    pcre_match_all("http://www.ics.uci.edu/pub/ietf/uri/#Related",
                       "^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\\?([^#]*))?(#(.*))?" ));

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

    There are 1 match in the entire string, the first item in the array is the matched substring, then all the
    captured groups.
