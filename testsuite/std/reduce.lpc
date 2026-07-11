//:FUNCTION reduce
//Iterate over an <arr>, passing each element to a function <fun>.
//The function should return a new value which becomes the new
//total. Function <fun> can be a function or a string. If it is a
//string, it will call the named function in the calling object
//(via previous_object()).
//
//Parameters:
//(required) mixed *elements - the array to be worked upon
//(required) function | string fun - the reducer function
//(optional) mixed initial - the initial value to set
//(optional) mixed args... - additional arguments to pass to <fun>
//
//
//The following is passed to the function at every iteration:
//(required) mixed total - The initial value, or the previously returned value of the function
//(required) mixed currentValue - The value of the current element
//(optional) int currentIndex - The array index of the current element
//(optional) mixed *arr - The array the current element belongs to
//(optional) mixed args... - Additional arguments
//
//The function should return what will become the new total.
//
//:EXAMPLE
//reduce( ({ 1, 2, 3, 4 }), function( int total, int current ) {
//    return total + current ;
//}) ;
////Result: 10
//
//reduce( ({ 1, 2, 3, 4 }), function( int total, int current ) {
//    return total + current ;
//}, 10) ;
////Result: 20
//
//Inspired by Javascript's Array.reduce()

varargs mixed reduce( mixed *elements, mixed fun, mixed initial, mixed args... )
{
    mixed result ;
    int num_elements, element ;

    if(nullp(elements)) error("Missing argument 1 to reduce.\n") ;
    if(nullp(fun)) error("Missing argument 2 to reduce.\n") ;

    result = initial ;

    if( functionp( fun ) )
    {
        for( element = 0, num_elements = sizeof( elements ); element < num_elements; element++ )
        {
            result = evaluate( fun, result, elements[ element ], element, elements, args... ) ;
        }
    }
    else if( stringp( fun ) )
    {
        object ob = previous_object() ;

        for( element = 0, num_elements = sizeof( elements ); element < num_elements; element++ )
        {
            result = call_other ( ob, fun, result, elements[ element ], element, elements, args... ) ;
        }
    }
    else
    {
        error("Bad argument 2 to reduce.\n") ;
    }

    return result ;
}
