// Test for unique_mapping
// By Yucong Sun (sunyucong@gmail.com)

void do_tests() {
    mixed* arr = ({ 1, 2, 3, 4, 5 });

	// standard behavior
    // key is in sorted order, and values in the list
    // is in reverse order.
    ASSERT_EQ(([ 0: ({ 4, 2 }), 1: ({ 5, 3, 1 }) ]),
      unique_mapping(arr, (: $1 % 2 :)));

    ASSERT_EQ(([ 0: ({ 5, 4, 3, 2, 1}) ]),
      unique_mapping(arr, (: 0 :)));

    // if the callback doesn't exist, result is grouped to 0
    ASSERT_EQ(([ 0: ({ 5, 4, 3, 2, 1}) ]),
      unique_mapping(arr, "none-exists"));

    // Old behavior, error in callback are passed through.
    ASSERT_EQ("whatever",
    		catch(unique_mapping(arr, (: throw("whatever") :))));

    arr = ({ this_player(), "123", arr });
    ASSERT_EQ(([ 0: ({ ({ 1, 2, 3, 4, 5 }), "123", this_player() }) ]),
      unique_mapping(arr, (: 0 :)));

    arr = ({ "alpha","bravo","charlie","apple","bongo","cat" });

    ASSERT_EQ( (["a": ({ "apple", "alpha" }),
                 "b": ({ "bongo", "bravo" }),
                 "c": ({ "cat", "charlie" }),
                ]),
               unique_mapping(arr, (: $1[0..0] :) ));

     ASSERT_EQ( 1, sizeof(
              unique_mapping( arr , (: this_object() :) ) ));

    // Crashers.
    ASSERT_EQ(6, sizeof(unique_mapping( arr , (: allocate(1) :) ) ));
    ASSERT_EQ(6, sizeof(unique_mapping( arr , (: allocate_mapping(1) :)) ));
}
