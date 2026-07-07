string* query_temp(string);
string query(string);

varargs string name(int raw)
{
  string str, *mask;

  if( !raw && sizeof(mask = query_temp("apply/name")) )  //这行报错
    return mask[sizeof(mask)-1];
  else
  {
    if( stringp(str = query("name")) )
      return str;
    else
      return file_name(this_object());
  }
}

string object_name() {
  return name();
}

void do_tests() {
  object* obs = objects();
  foreach(object ob in obs) {
    write(sprintf("%O: %d\n", ob, refs(ob)));
  }
}
