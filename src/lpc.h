#ifndef LPC_H
#define LPC_H

/* It is usually better to include "lpc_incl.h" instead of including this
   directly */

typedef struct {
    unsigned short ref;
} refed_t;

union u {
    const char *string;
    long number;
    double real;

    refed_t *refed; /* any of the block below */

#ifndef NO_BUFFER_TYPE
    struct buffer_s *buf;
#endif
    struct object_s *ob;
    struct array_s *arr;
    struct mapping_s *map;
    struct funptr_s *fp;

    struct svalue_s *lvalue;
    struct ref_s *ref;
    unsigned char *lvalue_byte;
    void (*error_handler) (void);
};

/*
 * The value stack element.
 * If it is a string, then the way that the string has been allocated
 * differently, which will affect how it should be freed.
 */
typedef struct svalue_s {
    short type;
    int subtype;
    union u u;
} svalue_t;

typedef struct ref_s {
    unsigned short ref;
    struct ref_s *next, *prev;
    struct control_stack_s *csp;
    svalue_t *lvalue;
    svalue_t sv;
} ref_t;

/* values for type field of svalue struct */
#define T_INVALID       0x0
#define T_LVALUE        0x1

#define T_NUMBER        0x2
#define T_STRING        0x4
#define T_REAL          0x80

#define T_ARRAY         0x8
#define T_OBJECT        0x10
#define T_MAPPING       0x20
#define T_FUNCTION      0x40
#ifndef NO_BUFFER_TYPE
#define T_BUFFER        0x100
#endif
#define T_CLASS         0x200

#define T_LVALUE_BYTE   0x400   /* byte-sized lvalue */
#define T_LVALUE_RANGE  0x800
#define T_ERROR_HANDLER 0x1000
#define T_FREED         0x2000
#define T_REF           0x4000

#ifdef NO_BUFFER_TYPE
#define T_REFED (T_ARRAY|T_OBJECT|T_MAPPING|T_FUNCTION|T_CLASS|T_REF)
#else
#define T_REFED (T_ARRAY|T_OBJECT|T_MAPPING|T_FUNCTION|T_BUFFER|T_CLASS|T_REF)
#endif
#define T_ANY (T_REFED|T_STRING|T_NUMBER|T_REAL)

/* values for subtype field of svalue struct */
#define STRING_COUNTED  0x1 /* has a length an ref count */
#define STRING_HASHED   0x2 /* is in the shared string table */

#define STRING_MALLOC   STRING_COUNTED
#define STRING_SHARED   (STRING_COUNTED | STRING_HASHED)
#define STRING_CONSTANT 0

#define T_UNDEFINED     0x4     /* undefinedp() returns true */

#endif
