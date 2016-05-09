/*
  Author: Zoilder (RL-Mud)
  Date: 04/05/2013
  Description: Functions for trim system.
*/

/*
  Args:
    string: the string to trim.
    string|void: the list of characters to remove. if not provided, remove all whitespaces (isspace() == 1).
*/

string trim(string, string|void);
string ltrim(string, string|void);
string rtrim(string, string|void);
