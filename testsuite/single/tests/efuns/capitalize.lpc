/*
          efun: string capitalize(string);
   test author: John Garnett
          date: 1993 November 2
*/

void do_tests()
{
    string str1, str2, str3;

    str1 = "";
    str2 = capitalize(str1);
    str3 = "";
    ASSERT(str2 == str3);

    str1 = "driver";
    str2 = capitalize(str1);
    str3 = "Driver";
    ASSERT(str2 == str3);
    ASSERT(str1 == "driver");
    
    str1 = "Driver";
    str2 = capitalize(str1);
    str3 = "Driver";
    ASSERT(str2 == str3);
    ASSERT(str1 == "Driver");

    str1 = "42driver";
    str2 = capitalize(str1);
    str3 = "42driver";
    ASSERT(str2 == str3);
    ASSERT(str1 == "42driver");
}
