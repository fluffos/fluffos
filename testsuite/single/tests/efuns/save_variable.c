#define IS(x, y) ASSERT_EQ(y, save_variable(x))

mapping values = 
([
  "\n" : "\"\r\"",
  "\"" : "\"\\\"\"",
  "\"x\\\n" : "\"\\\"x\\\\\r\"",
  -1 : "-1",
  0 : "0",
  1 : "1",
  22 : "22",
  1.2 : "1.200000",
  333 : "333",
  ({}) : "({})",
  ({ 0 }) : "({0,})",
  // Special cases for encoding '\r'
  "a\rb\rc\rd\r\n\radsfasdsdf\r" : "\"a\\\rb\\\rc\\\rd\\\r\r\\\radsfasdsdf\\\r\"",
]);

void do_tests() {
    mixed x, y;
    foreach (x, y in values) {
	IS(x, y);
    }
    
    // arrays
    foreach (x, y in values) {
	IS( ({ x }), "({" + y + ",})");
	IS( ({ x, x }), "({" + y + "," + y + ",})");
    }
    
    // mappings
    foreach (x, y in values) {
	IS( ([ 1 : x ]), "([1:" + y + ",])");
	IS( ([ x : x ]), "([" + y + ":" + y + ",])");
    }
}    
