# ===========================================================================
#       http://www.gnu.org/software/autoconf-archive/ax_show_config_info.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_SHOW_CONFIG_INFO()
#
# DESCRIPTION
#
#   Show a summary of configuration values
#
#   This macro calls:
#
#     AS_ECHO(...)
#     AS_HELP_STRING(...)
#
# LICENSE
#
#   Copyright (c) 2016 Rene Mueller
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([AX_SHOW_CONFIG_INFO],
[
    AS_ECHO([========================================])
    AS_ECHO([Directory Layout:])
    m4_foreach_w([x],
        [prefix exec_prefix bindir sbindir libdir libexecdir datarootdir datadir sysconfdir localstatedir sharedstatedir localedir mandir infodir docdir dvidir htmldir pdfdir psdir includedir oldincludedir],
        [AS_ECHO("AS_HELP_STRING([x:], [$x])")
        ])
    AS_ECHO([========================================])
])
