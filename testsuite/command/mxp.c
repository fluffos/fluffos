int main(string arg)
{
  int mxp_enabled = has_mxp(this_player());

  write("MXP status: " + mxp_enabled + "\n");

  if(mxp_enabled) {
    write("Entering MXP mode:\n");
    act_mxp();
  }

  return 1;
}
