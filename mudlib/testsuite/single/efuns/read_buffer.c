/*
          efun: mixed read_buffer(string|buffer, void|int, void|int);
   test author: John Garnett
          date: 1993 November 8
*/

inherit "/inherit/tests";

#define TEST_STRING "ABC"

buffer b;
int i;

int
test(int which)
{
	string s;

	switch (which) {
	case 0:
		describe_test("string read_buffer(buffer b)");
		s = read_buffer(b);
		return (s == TEST_STRING);
	break;
	case 1:
		describe_test("string read_buffer(buffer b, 0)");
		s = read_buffer(b);
		return (s == TEST_STRING);
	break;
	case 2:
		describe_test("string read_buffer(buffer b, sizeof(b)-1)");
		s = read_buffer(b, sizeof(b)-1);
		return (s == TEST_STRING[i..i]);
	break;
	case 3:
		describe_test("string read_buffer(buffer b, sizeof(b))");
		s = read_buffer(b, sizeof(b));
		return (s == 0);
	break;
	case 4:
		describe_test("string read_buffer(buffer b, -sizeof(buffer))");
		s = read_buffer(b, -sizeof(b));
		return (s == TEST_STRING);
	break;
	case 5:
		describe_test("string read_buffer(buffer b, -(sizeof(b)+1))");
		s = read_buffer(b, -(sizeof(b) + 1));
		return (s == 0);
	break;
	case 6:
		describe_test("string read_buffer(buffer b, 1, 1)");
		s = read_buffer(b, 1, 1);
		return (s == TEST_STRING[1..1]);
	break;
	case 7:
		describe_test("string read_buffer(buffer b, 1, sizeof(b))");
		s = read_buffer(b, 1, sizeof(b));
		return (s == TEST_STRING[1..i]);
	break;

	/* add cases here to handle reading from files */

	default:
	return -1;
	break;
	}
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
