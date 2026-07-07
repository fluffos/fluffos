/**
 * NAME: number_string
 * SYNOPSIS: string number_string ( int | float num1 number, void | int add_commas )
 * DESCRIPTION: Converts an integer or a float into a string. Optionally, can comma-delimit the results.
 * EXAMPLE: number_string(2500.36) // "2500.360000"
 * EXAMPLE: number_string(2500.36, 1) // "2,500.360000"
 */

#include <lpctypes.h>

varargs string number_string( mixed number, int add_commas )
{
    string int_part = "", decimal_part = "", *parts, part ;
    string work ;

    if(nullp(number) || ( typeof(number) != INT && typeof(number) != FLOAT ) )
    {
        error("You must specify a number of type int or float.\n") ;
    }

    work = number + "" ;

    if( !nullp(add_commas) || add_commas == 1 )
    {
        int decimal_index = strsrch(work, ".") ;

        if(decimal_index > -1)
        {
            int_part = work[ 0 .. decimal_index - 1] ;
            decimal_part = work[ decimal_index .. ] ;
        }
        else
        {
            int_part = work ;
        }

        // empty work
        work = "" ;

        while( strlen( int_part ) > 3 )
        {
            work = "," + int_part[ <3 .. ] + work ;
            int_part = int_part[ 0 .. <4 ] ;
        }

        int_part = int_part + work ;
    }
    else
    {
        int_part = work ;
    }

    return sprintf("%s%s", int_part, decimal_part );
}
