---
layout: default
title: http / curl_free
---

### NAME

    curl_free() - Free's a curl response handle for use again

### SYNOPSIS

    int curl_free( int );

### DESCRIPTION

    Frees the results of a curl request. This must be done for each request or memory will leak!

### SEE ALSO

    curl_get_status_code(3), curl_get_body(3), curl_get_headers(3), curl_free(3)

