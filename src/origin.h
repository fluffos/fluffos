#ifndef ORIGIN_H
#define ORIGIN_H

enum origin { 
    ORIGIN_DRIVER = 0x01, 
    ORIGIN_LOCAL = 0x02, 
    ORIGIN_CALL_OTHER = 0x04, 
    ORIGIN_SIMUL_EFUN = 0x08,
    ORIGIN_INTERNAL = 0x10,
    ORIGIN_EFUN = 0x20,         
    /* pseudo frames for call_other function pointers and efun pointer */
    ORIGIN_FUNCTION_POINTER = 0x40,
    /* anonymous functions */
    ORIGIN_FUNCTIONAL = 0x80
};

#endif
