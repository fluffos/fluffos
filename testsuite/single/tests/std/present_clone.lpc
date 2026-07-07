void do_tests()
{
  string container_file = "/clone/testob1" ;
  string second_object_file = "/clone/testob2" ;
  string third_object_file = "/clone/testob3" ;

  // Create a container object to hold the test objects
  object container = new( container_file ) ;

  // Create two of the testob2
  object first2 = new( second_object_file ) ;
  object second2 = new( second_object_file ) ;

  // Create two of the testob3
  object first3 = new( third_object_file ) ;
  object second3 = new( third_object_file ) ;

  // Move them to the container
  second2->move(container) ;
  first2->move(container) ;
  second3->move(container) ;
  first3->move(container) ;

  //Test for invalid thing in the container, should return 0
  ASSERT_EQ( present_clone("nothing", container), 0) ;

  //by string
  ASSERT_EQ( first2, present_clone(second_object_file, container) ) ;
  ASSERT_EQ( second2, present_clone(second_object_file, container, 1) ) ;
  ASSERT_EQ( 0, present_clone(second_object_file, container, 2) ) ;

  //by object
  ASSERT_EQ( first2, present_clone(first2, container) ) ;
  ASSERT_EQ( second2, present_clone(first2, container, 1) ) ;
  ASSERT_EQ( 0, present_clone(first2, container, 2) ) ;

  //by string
  ASSERT_EQ( first3, present_clone(third_object_file, container) ) ;
  ASSERT_EQ( second3, present_clone(third_object_file, container, 1) ) ;
  ASSERT_EQ( 0, present_clone(third_object_file, container, 2) ) ;

  //by object
  ASSERT_EQ( first3, present_clone(first3, container) ) ;
  ASSERT_EQ( second3, present_clone(first3, container, 1) ) ;
  ASSERT_EQ( 0, present_clone(first3, container, 2) ) ;

  // Clean up after ourselves
  destruct(first2) ;
  destruct(second2) ;
  destruct(first3) ;
  destruct(second3) ;
  destruct(container) ;
}
