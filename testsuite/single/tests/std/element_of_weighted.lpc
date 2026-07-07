// Test ELEMENT_OF_WEIGHTED sefun

void do_tests()
{
    mapping data = ([
        "element0" : 0,
        "element1" : 1,
        "element2" : 2,
    ]) ;
    string element, *elements = keys(data) ;
    
    for (int i = 0; i < 100; i++) {
        element = element_of_weighted( data ) ;
        
	    ASSERT( member_array( element , elements ) > -1 ) ;
	    ASSERT( element != "element0" );
    }
}
