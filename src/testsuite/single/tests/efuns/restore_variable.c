#define ERR(x) ASSERT2(catch(restore_variable(x)), sprintf("%O", x))
#define IS(x, y) ASSERT2(same(restore_variable(x),y), sprintf("%O", x))

string *value_errs = ({ "\"\"x", "\"\\", "\"\\x\\", "\"\\x\"x",
		        "-x" });
mapping values = ([
    "\"\r\"" : "\n",
    "\"\\\"\"" : "\"",
    "\"\\\"\\x\r\"" : "\"x\n",
    "-1" : -1,
    "0" : 0,
    "1" : 1,
    "22" : 22,
    "1.2" : 1.2, 
    "333" : 333,
    "({})" : ({}),
    "({,})" : ({0})
]);

void do_tests() {
    mixed x, y;
    foreach (x in value_errs) {
	ERR(x);
    }

    foreach (x, y in values) {
	IS(x, y);
    }

    // arrays
    foreach (x, y in values) {
	IS("({" + x + ",})", ({ y }));
	IS("({" + x + "," + x + ",})", ({ y, y }));
    }
    
    // mappings
    foreach (x, y in values) {
	IS("([1:" + x + ",])", ([1:y ]));
	IS("([" + x + ":" + x + ",])", ([ y: y ]));
    }
}
