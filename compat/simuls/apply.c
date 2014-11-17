/* Like evaluate(), but expand out an array of arguments.  No longer necessary
   because of the ... syntax */

mixed apply(mixed f, mixed *arr...) {
    if (sizeof(arr) && arrayp(arr[<1]))
        arr = arr[0..<2] + arr[<1];
    
    return evaluate(f, arr...);
}
