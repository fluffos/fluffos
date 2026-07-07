/**
 * NAME: highest
 * SYNOPSIS: (int | float) highest( int | float num1, int | float num2, int | float numx... )
 * DESCRIPTION: Given a number of integers and/or floats, returns the highest value.
 */

#include <lpctypes.h>

mixed highest(mixed *numbers...)
{
    mixed number, result ;

    if( sizeof(numbers) == 0 ) error("Missing at least one argument of type int or float.\n");

    // seed result with the first element for comparison
    result = numbers[0];

    foreach(number in numbers)
    {
        switch(typeof(number))
        {
            case INT : case FLOAT :
                result = number > result ? number : result ;
                break ;
            default: error("All argument values must be of type int or float.\n");
        }
    }

    return result ;
}
