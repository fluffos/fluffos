/*
  Author: Zoilder (RL-Mud)
  Date: 04/05/2013
  Description: Functions for trim system.
*/
#include "spec.h"

/*
  Parameter 1: Is mixed, so if it isn't string, it returns "".
  Parameter 2: Could be string, with a list of characters leading / trailing to remove. Void use isspace function.
*/
string trim(string, string | void);
string ltrim(string, string | void);
string rtrim(string, string | void);
