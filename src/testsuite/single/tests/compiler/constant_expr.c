void do_tests() {
    mixed code;

    // array constants
    code = __TREE__ ( sizeof( ({ 1, 3, 3 }) ) );
    ASSERT(same(code, ({ "number", 3 })));

    code = __TREE__ ( ({ 1, 3, 3 })[0] );
    ASSERT(same(code, ({ "number", 1 })));

    code = __TREE__ ( ({ 1, 4, 9 })[<2] );
    ASSERT(same(code, ({ "number", 4 })));

    code = __TREE__ { int i; for (; i < 5; i++) ; };
    ASSERT(same(code, ({
	"two values", 0, ({
	    "loop", 1, 0, ({
		"opcode_2", "loop_cond_number", 1, 5 
	    }), ({ 
		"opcode_1", "loop_incr", 1 
	    }) }) }) ));

    code = __TREE__( !code ? 1 : 2 );
    ASSERT(same(code, ({
	"if", ({
	    "number", 2 
	}), ({
	    "number", 1
	}), ({
	    "opcode_1", "local", 0
	}) }) ));

    ASSERT(5/3 == 1);
    ASSERT(1/8.0 == 0.125);
    ASSERT(1.0/2 == 0.5);
    ASSERT(1.0/2.0 == 0.5);
}
