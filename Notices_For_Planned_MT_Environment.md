# fliconv.cc #

* possible race condition with several static variables

* possible problems with call to iconv(3):

    size_t iconv(iconv_t cd, char **inbuf, size_t *inbytesleft, char **outbuf, size_t *outbytesleft);

    The iconv() function is MT-Safe, as long as callers arrange for mutual exclusion on the cd argument.
