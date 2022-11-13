---
layout: default
title: external / external_start
---

### NAME

    external_start() - execute a shell command external to the driver

### SYNOPSIS

    int external_start(int external_index,
                       string *args,
                       string|function read_call_back,
                       string|function write_call_back,
                       string|function close_call_back)

### DESCRIPTION

    Execute a shell command external to the driver. 

    Commands that you would like to execute must be added to the runtime config.
    The enumerated commands may then be invoked by their number as the first
    argument to external_start `external_index`. 

    This function returns the socket number which you should record for later
    processing of the results from the external command.

    `args` - An array of the arguments passed to the external command.
    `read_call_back` - As data becomes available, this function will be called
    with a string containing that data.
    `write_call_back` - I am not 100% sure what would be written to the external
    command, but, this is a required parameter.
    `close_call_back` - When the socket closes, this function is called.

### RUNTIME CONFIGURATION

    You can configure your runtime configuration to know about various external
    commands by adding lines such as: external_cmd_## : /path/to/command where
    ## is the number that will be invoked with `external_start`.

        # external commands
        external_cmd_1 : /home/gesslar/.nvm/versions/node/v17.4.0/bin/node
        external_cmd_2 : /usr/bin/curl

### CALLBACKS

    The callbacks, when invoked, will pass the following information:

        void read_call_back(int fd, string data)
        void write_call_back(int fd)
        void close_call_back(int fd)

### EXAMPLE

    This is a very basic example using the information provided in this document

```c
// curl.c

#define CURL_CMD 2

void read_call_back(int, string) ;
void write_call_back(int) ;
void close_call_back(int) ;

private nosave mapping fds = ([ ]) ;
private nosave string *base_args = ({ "-N", "--no-progress-meter", }) ;

void fetch(object caller, string call_back, mixed *args...) {
    int fd ;

    if(sizeof(args)) args = ({ base_args..., args..., }) ;
    else args = base_args ;

    fd = external_start(CURL_CMD,
        args, 
        (: read_call_back :),
        (: write_call_back :),
        (: close_call_back :)
    );

    // Unable to bind to external command
    if(fd < 0) return ;

    fds[fd] = ([
        "caller" : caller,
        "call_back" : call_back,
        "data" : "",
        "start_time" : perf_counter_ns(),
    ]) ;
}

void read_call_back(int fd, string mess) {
    fds[fd]["data"] += mess ;
}

void write_call_back(int fd) {
    // n/a
}

void close_call_back(int fd) {
    object caller = fds[fd]["caller"] ;
    string call_back = fds[fd]["call_back"] ;
    string result = fds[fd]["data"] ;
    int started = fds[fd]["start_time"] ;
    int ended = perf_counter_ns() ; 

    map_delete(fds, fd) ;

    if(!objectp(caller)) return ;
    if(!function_exists(call_back, caller)) return ;

    call_other(caller, call_back, result) ;
}

```

### ADDITIONAL INFORMATION

    This efun requires that PACKAGE_EXTERNAL be compiled into the driver.
