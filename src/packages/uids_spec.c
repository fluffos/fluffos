#include "spec.h"

int export_uid(object);
string geteuid(function | object default:F_THIS_OBJECT);
string getuid(object default:F_THIS_OBJECT);
int seteuid(string | int);
