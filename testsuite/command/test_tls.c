#define STD_HTTP "/std/http.c"

object tp;

void callback(object tp, string result)
{
  tell_object(tp, "Result: \n" + result + "\n");
}

int main(string arg)
{
  tp = this_player();
  STD_HTTP->get("www.google.com", 443, "/", 1, (: callback(tp, $1) :));
  return 1;
}
