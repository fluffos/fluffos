// mudlib: Lil
// file:   /single/simul_efun.c

#pragma show_error_context
#include <globals.h>

void
cat(string file)
{
	write(read_file(file));
}

varargs int
getoid(object ob)
{
	int id;

	sscanf(file_name(ob || previous_object()), "%*s#%d", id);
	return id;
}

string
user_cwd(string name)
{
   return ("/u/" + name[0..0] + "/" + name);
}

string user_path(string name)
{
   return (user_cwd(name) + "/");
}

// Get the owner of a file.  Used by log_error() in master.c.

string
file_owner(string file)
{
    string temp;
    
    if (file[0] != '/') file = "/" + file;

    if (sscanf(file, "/u/%s/%s/%*s", temp, temp) == 2) {
        return temp;
    }
    return 0;
}

#include <lpctypes.h>

// dump_variable, author: Huthar@Portals, TMI
// - returns a printable representation of any variable.

string
dump_variable(mixed arg)
{
   mixed *index;
   string rtn;
   int i;

   switch (typeof(arg)) {
   case T_OBJECT: return "(" + file_name(arg) + ")";
   case T_STRING: return "\"" + arg + "\"";
   case T_NUMBER: return "#" + arg;
   case T_ARRAY:
       {
	   int j;

	   rnt = "ARRAY\n";
	   i = sizeof(arg);
	   while (j < i) 
	       rtn += sprintf("[%d] == %s\n", j, dump_variable(arg[j++]));
	   return rtn;
       }
   case T_MAPPING:
       {
	   return "MAPPING\n" + implode(map_array(keys(arg), (: sprintf("[%s] == %s", dump_variable($1), ($(arg)[$1]) ) :)), "\n");
       }

   case T_FUNCTION:
   case T_CLASS:
   case T_REAL:
   case T_BUFFER:
       /* this really could be done for all of them, but the above format
	  is traditional */
       return sprintf("%O\n", arg);
   }

   return "UNKNOWN";
}

/*
// Thanks to Huthar for resolve_path.
// Rewrite by Symmetry 5/4/95
*/

string resolve_path(string curr, string newer) {
    int i, j, size;
    string *tmp;
    
    switch(newer){
    case 0: 
    case ".":
	return curr;
	
    case "here":
	return file_name(environment())+".c";
	
    default:
	if (newer[0..1] == "~/") newer = user_path((string)this_player()->query_name()) + newer[2..];
	else {
	    switch(newer[0]){
	    case '~': newer = user_path(newer[1..]); break;
	    case '/': break;
	    default: newer[<0..<1] = curr + "/";
	    }
	}
	
	if (newer[<1] != '/') newer += "/";
	size = sizeof(tmp = regexp(explode(newer, "/"), "."));
	
	i = j = 0;
	
	while (i < size){
	    switch(tmp[i]){
	    case "..":
		if (j){
		    while (j-- && !tmp[j]);
		    if (j >= 0) tmp[j] = 0;
		    else j++;
		}
	    case ".":
		tmp[i++] = 0;
		break;
		
	    default:
		j = ++i;
		break;
	    }
	}
	return "/"+implode(tmp, "/");
    }
}


// domain_file should return the domain associated with a given file.

string
domain_file(string file)
{
	return ROOT_UID;
}

// creator_file should return the name of the creator of a specific file.

string
creator_file(string file)
{
	return ROOT_UID;
}

// author_file should return the name of the author of a specific file.

string
author_file(string file)
{
	return ROOT_UID;
}
