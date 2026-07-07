int main(string arg)
{
  int msdp_enabled = has_msdp(this_player());

  write("MSDP status: " + msdp_enabled + "\n");

  if(msdp_enabled) {
    write("MSDP sending int named hp value of 100.");
    this_player()->send_msdp_variable("hp", 100);
    write("MSDP sending name named name value of name.");
    this_player()->send_msdp_variable("name", "name");
    write("MSDP sending float named hp value of 100.0.");
    this_player()->send_msdp_variable("hp", 100.0);
    write("MSDP sending buffer named name value of name.");
    mixed buff = allocate_buffer(4);
    write_buffer(buff, 0, "name");
    this_player()->send_msdp_variable("name", buff);
  }

  return 1;
}
