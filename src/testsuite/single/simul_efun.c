// mudlib: Lil
// file:   /single/simul_efun.c

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

	if (!ob) {
		ob = previous_object();
	}
	sscanf(file_name(ob), "%*s#%d", id);
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
    string name, rest, dir;

    if (file[0] != '/') {
        file = "/" + file;
    }
    if (sscanf(file, "/u/%s/%s/%s", dir, name, rest) == 3) {
        return name;
    }
    return 0;
}

// dump_variable, author: Huthar@Portals, TMI
// - returns a printable representation of any variable.

string
dump_variable(mixed arg)
{
   mixed *index;
   string rtn;
   int i;
   
   if (objectp(arg))
      return "("+file_name(arg)+")";
   
   if (stringp(arg))
      return "\""+arg+"\"";
   
   if (intp(arg))
      return "#"+arg;
   
   if (pointerp(arg)) {
      rtn = "ARRAY\n";
      
      for (i = 0; i < sizeof(arg); i++)
         rtn += "["+i+"] == "+dump_variable(arg[i])+"\n";
      
      return rtn;
   }

   if (mapp(arg)) {
      rtn = "MAPPING\n";

      index = keys(arg);

      for (i = 0; i < sizeof(index); i++)
         rtn += "[" + dump_variable(index[i]) + "] == "
            + dump_variable(arg[index[i]])+"\n";
      return rtn;
   }

   if (functionp(arg)) {
      rtn = "FUNCTION\n";
      return "(:" + dump_variable(arg[0]) + ", "
        + dump_variable(arg[1]) + ":)\n";
   }

   return "UNKNOWN";
}

/*
// Thanks to Huthar for resolve_path.
*/

string resolve_path(string curr, string new) {
    int i;
    string *tmp;
    string t1,t2,t3,t4;

    if (!new || new == ".") return curr;
    if (new == "here")
    {
        return file_name(environment(this_object())) + ".c";
    }
    if (new == "~" || new == "~/" )
      new = user_path((string)this_player()->query_name());
    if (sscanf(new,"~/%s",t1))
      new = user_path((string)this_player()->query_name()) + t1;
    else if (sscanf(new,"~%s",t1))
      new = user_path(t1); 
    else if (new[0] != '/')
      new = curr + "/" + new;

    if (new[strlen(new) - 1] != '/')
        new += "/";
    tmp = explode(new,"/");
    if (!tmp) tmp = ({"/"});
    for (i = 0; i < sizeof(tmp); i++)
        if (tmp[i] == "..") {
            if (sizeof(tmp) > 2) {
                tmp = tmp[0..(i-2)] + tmp[(i+1)..(sizeof(tmp)-1)];
                i -= 2;
            } else {
                tmp = tmp[2 ..(sizeof(tmp)-1)];
                i = 0;
            }
        }
     new = "/" + implode(tmp,"/");
     if (new == "//") new = "/";
     return new;
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
