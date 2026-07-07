/*
          efun: mixed read_buffer(string|buffer, void|int, void|int);
   test author: John Garnett
          date: 1993 November 8
*/

#define TEST_STRING "ABC"

buffer b;
int i;

void do_tests() {
    string s;

    s = read_buffer(b);
    ASSERT(s == TEST_STRING);

    s = read_buffer(b, sizeof(b)-1);
    ASSERT(s == TEST_STRING[i..i]);

    s = read_buffer(b, sizeof(b));
    ASSERT(s == 0);

    s = read_buffer(b, -sizeof(b));
    ASSERT(s == TEST_STRING);

    s = read_buffer(b, -(sizeof(b) + 1));
    ASSERT(s == 0);

    s = read_buffer(b, 1, 1);
    ASSERT(s == TEST_STRING[1..1]);

    s = read_buffer(b, 1, sizeof(b));
    ASSERT(s == TEST_STRING[1..i]);

    /* add cases here to handle reading from files */
}

void
create()
{
	int j, limit;

	i = strlen(TEST_STRING) - 1;
	limit = strlen(TEST_STRING);
	b = allocate_buffer(limit);
	for (j = 0; j < limit; j++) {
	    b[j] = TEST_STRING[j];
	}
}
