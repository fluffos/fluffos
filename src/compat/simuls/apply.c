/* Like evaluate(), but expand out an array of arguments.  No longer necessary
   because of the ... syntax */

mixed apply(mixed f, mixed *arr) {
    return evaluate(f, arr...);
}
