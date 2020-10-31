/**
 * NAME: sum
 * SYNOPSIS: (int|float) sum( int|float num1, int|float num2, int|float numx... )
 * DESCRIPTION: Returns the sum of all passed values.
 */

#include <lpctypes.h>

mixed sum(mixed *numbers...)
{ 
    mixed number, result = 0;

    foreach(number in numbers)
    {
        switch(typeof(number))
        {
            case INT : 
                result += number ;
                break ;
            case FLOAT :
                result = to_float(result) + number ;
                break ;
            default:
                error("Argument must be an array of int or float.\n");
        }
        
    } 

    return result; 
}

/**
 * NAME: array_sum
 * SYNOPSIS: (int|float) array_sum( array int|array float numbers  )
 * DESCRIPTION: Returns the sum of all values in the passed array.
 */

mixed array_sum(mixed *numbers)
{ 
    return sum(numbers...) ;
}
