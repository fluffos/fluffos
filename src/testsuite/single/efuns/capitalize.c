/*
          efun: string capitalize(string);
   test author: John Garnett
          date: 1993 November 2
*/

inherit "/inherit/tests";

int test(int which)
{
	string str1, str2, str3;

	switch (which) {
	case 0:
		describe_test("empty string");
		str1 = "";
		str2 = capitalize(str1);
		str3 = "";
		return (str2 == str3);
	break;

	case 1:
		describe_test("lowercase");
		str1 = "driver";
		str2 = capitalize(str1);
		str3 = "Driver";
		return (str2 == str3);
	break;

	case 2:
		describe_test("uppercase");
		str1 = "Driver";
		str2 = capitalize(str1);
		str3 = "Driver";
		return (str2 == str3);
	break;

	case 3:
		describe_test("numeric");
		str1 = "42driver";
		str2 = capitalize(str1);
		str3 = "42driver";
		return (str2 == str3);
	break;
	default:
	return -1;
	break;
	}
}
