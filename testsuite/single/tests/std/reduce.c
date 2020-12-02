// Test REDUCE sefun

void do_tests()
{
    int *numbers = ({ 1, 2, 3, 4 }) ;
    int *result = ({ }) ;

    // Testing with a local function
    ASSERT_EQ( reduce( numbers, "reducer", 0 ), 10 ) ;

    // Testing with a function
    ASSERT_EQ( reduce( numbers, function(int acc, int elem) {
        return acc + elem ;
    }, 0), 10 ) ;

    // Testing with a functional
    ASSERT_EQ( reduce( numbers, (: $1 + $2 :), 0), 10)

    // Testing with a functional
    ASSERT_EQ( reduce( numbers, (: $1 + $2 :), ""), "1234" );

    //Testing using index
    ASSERT_EQ( reduce( numbers, function( int total, int current, int idx, int* arr ) {
        return total + current + idx ;
    }, 0), 16 );

    //Testing using index and array
    ASSERT_EQ( reduce( explode("FluffOS", ""), function( string total, string current, int idx, string* arr ) {
        return total + arr[sizeof(arr) - idx - 1] ;
    }, ""), "SOffulF") ;

    ASSERT_EQ( reduce( ({ "north", "south", "east", "west" }), function( string total, string current, int idx, string* arr, string conjunction ) {
        if( idx == 0 ) return current ;

        total += ", " ;

        if( idx == sizeof( arr ) - 1 ) total += conjunction + " " ;
        
        return total + current ;
    }, "", "or"), "north, south, east, or west") ;
}

int reducer( int accumulated, int element )
{
    return accumulated + element ;
}
