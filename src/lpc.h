#ifndef LPC_H
#define LPC_H

typedef struct {
    unsigned short ref;
} refed_t;

union u {
    char *string;
    int number;
    float real;

    refed_t *refed; /* any of the block below */

    struct buffer_s *buf;
    struct object_s *ob;
    struct array_s *arr;
    struct mapping_s *map;
    struct funptr_s *fp;

    struct svalue_s *lvalue;
    unsigned char *lvalue_byte;
    void (*error_handler) PROT((void));
};

/*
 * The value stack element.
 * If it is a string, then the way that the string has been allocated
 * differently, which will affect how it should be freed.
 */
typedef struct svalue_s {
    short type;
    short subtype;
    union u u;
} svalue_t;

/* values for type field of svalue struct */
#define T_INVALID	0x0
#define T_LVALUE	0x1

#define T_NUMBER	0x2
#define T_STRING	0x4
#define T_REAL          0x80

#define T_ARRAY  	0x8
#define T_OBJECT	0x10
#define T_MAPPING	0x20
#define T_FUNCTION      0x40
#define T_BUFFER        0x100
#define T_CLASS         0x200

#define T_REFED (T_ARRAY|T_OBJECT|T_MAPPING|T_FUNCTION|T_BUFFER|T_CLASS)

#define T_LVALUE_BYTE   0x400	/* byte-sized lvalue */
#define T_LVALUE_RANGE  0x800
#define T_ERROR_HANDLER 0x1000
#ifdef DEBUG
#define T_FREED         0x2000
#endif
#define T_ANY T_STRING|T_NUMBER|T_ARRAY|T_OBJECT|T_MAPPING|T_FUNCTION| \
	T_REAL|T_BUFFER|T_CLASS

/* values for subtype field of svalue struct */
#define STRING_COUNTED  0x1 /* has a length an ref count */
#define STRING_HASHED   0x2 /* is in the shared string table */

#define STRING_MALLOC   STRING_COUNTED
#define STRING_SHARED   (STRING_COUNTED | STRING_HASHED)
#define STRING_CONSTANT 0

#define T_UNDEFINED     0x4	/* undefinedp() returns true */

#endif
