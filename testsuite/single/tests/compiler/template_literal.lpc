void do_tests() {
  string name;
  int count;
  float price;
  string result;

  // Basic template literal with no interpolation
  result = `hello world`;
  ASSERT_EQ(result, "hello world");

  // Simple variable interpolation
  name = "Alice";
  result = `Hello, ${name}!`;
  ASSERT_EQ(result, "Hello, Alice!");

  // Integer interpolation
  count = 42;
  result = `You have ${count} items.`;
  ASSERT_EQ(result, "You have 42 items.");

  // Float interpolation (should use %g coercion)
  price = 3.14;
  result = `Price: ${price}`;
  ASSERT_EQ(result, "Price: 3.14");

  // Multiple interpolations
  name = "Bob";
  count = 5;
  result = `${name} has ${count} apples.`;
  ASSERT_EQ(result, "Bob has 5 apples.");

  // Expression in interpolation
  count = 10;
  result = `Double: ${count + count}`;
  ASSERT_EQ(result, "Double: 20");

  // Empty interpolation parts
  name = "World";
  result = `${name}`;
  ASSERT_EQ(result, "World");

  // Escape sequences
  result = `hello\tworld`;
  ASSERT_EQ(result, "hello\tworld");

  // Escaped backtick
  result = `hello \` world`;
  ASSERT_EQ(result, "hello ` world");

  // Escaped dollar-brace
  result = `hello \${name} world`;
  ASSERT_EQ(result, "hello ${name} world");

  // Adjacent template literals collapse (like strings)
  result = `hello`
  `world`;
  ASSERT_EQ(result, "helloworld");

  // Adjacent: template with interpolation + plain template
  name = "X";
  result = `a${name}b`
  `cd`;
  ASSERT_EQ(result, "aXbcd");

  // Adjacent: template with interpolation + template with interpolation
  count = 1;
  result = `${name}` `${count}`;
  ASSERT_EQ(result, "X1");

  // Adjacent: string + template with interpolation
  result = "hi " `${name}!`;
  ASSERT_EQ(result, "hi X!");

  // Adjacent: template with interpolation + string
  result = `${name}: ` "done";
  ASSERT_EQ(result, "X: done");

  // Function call in interpolation (no string arg)
  count = 4;
  result = `${count + 0} chars`;
  ASSERT_EQ(result, "4 chars");

  // Function call with parens
  result = `len: ${strlen("hi")}`;
  ASSERT_EQ(result, "len: 2");

  // Function call with string arg and text after
  result = `${strlen("hi")} chars`;
  ASSERT_EQ(result, "2 chars");

  // Template literal + template literal
  name = "A";
  count = 1;
  result = `${name}` + `${count}`;
  ASSERT_EQ(result, "A1");

  // Template literal + string
  result = `hi ${name}` + " there";
  ASSERT_EQ(result, "hi A there");

  // Adjacent template literals (no interpolation)
  result = `foo` `bar`;
  ASSERT_EQ(result, "foobar");

  // --- Regression tests from a self-review pass vs master ---

  // Array literal inside the interpolation: "({"  is one lexer token whose
  // closing '}' is a separate bare token -- it must count toward brace
  // depth or the template garbles after it.
  result = `first: ${ ({ "a", "b" })[0] }!`;
  ASSERT_EQ(result, "first: a!");

  // Comment-lookalikes inside template text: the standalone preprocessor
  // used to strip these as real comments (a URL ate the rest of the line,
  // closing backtick included).
  result = `visit http://example.com/ now`;
  ASSERT_EQ(result, "visit http://example.com/ now");
  result = `not /* a comment */ here`;
  ASSERT_EQ(result, "not /* a comment */ here");

  // Macro expansion boundary: template *text* is protected string content
  // (TL_WORD below must NOT expand there), while ${...} interpolations get
  // full macro expansion (matching how "..." literals vs code behave).
#define TL_WORD "expanded"
  result = `say TL_WORD is ${TL_WORD}`;
  ASSERT_EQ(result, "say TL_WORD is expanded");
#undef TL_WORD

  // Nested template with comment-lookalikes at both text levels plus an
  // array literal, all in one -- the combined stress shape.
  result = `a//b ${ `x//y ${ ({ 1, 2 })[1] } z` } c`;
  ASSERT_EQ(result, "a//b x//y 2 z c");

  // Bug 7: Template literal as a macro argument
#define ID(x) (x)
#define PAIR(a, b) (b)
  result = ID(`a,b`);
  ASSERT_EQ(result, "a,b");
  result = PAIR("x", `n=${PAIR(1, 2)}`);
  ASSERT_EQ(result, "n=2");
#undef ID
#undef PAIR
}
