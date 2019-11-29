#define ERR(i, x) ASSERT2(catch(restore_variable(x)), sprintf("Unexpected successful restore idx %d,\n %O\n", i, x))
#define IS(x, y) ASSERT_EQ(y, restore_variable(x))

string *value_errs = ({ "\"\"x", "\"\\", "\"\\x\\", "\"\\x\"x", "-x", "\"", "\"\\", "\"\r\"\r" });
mapping values = ([
    "\"\r\"" : "\n",
    "\"\\\"\"" : "\"",
    "\"\\\"\\x\r\"" : "\"x\n",
    // Special cases for encoding '\r'
    "\"a\\\rb\\\rc\\\rd\\\r\r\\\radsfasdsdf\\\r\"": "a\rb\rc\rd\r\n\radsfasdsdf\r",
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
    int i = 0 ;
    for (i = 0; i < sizeof(value_errs); i++) {
	  ERR(i, value_errs[i]);
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
