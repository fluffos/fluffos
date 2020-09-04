int main(string arg)
{
  int gmcp_enabled = has_gmcp(this_player());

  write("GMCP status: " + gmcp_enabled + "\n");

  if(gmcp_enabled) {
    mapping char_info = ([
    "hp": "100",
        "maxhp": "100",
    ]);
    string res;

    res = "Char.Vitals " + json_encode(char_info) + "\n";
    write("GMCP sending: " + res);
    send_gmcp(res);
  }

  return 1;
}
