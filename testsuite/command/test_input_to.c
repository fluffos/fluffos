string* res = ({ });
nosave object receiver;

void create() {
  restore_object("/1.o");
}

void on_input(string item) {
  res += ({ item });
  tell_object(receiver, sprintf("1: %O\n", res));
  save_object("/1.o");
  tell_object(receiver, sprintf("2: %O\n", res));
  restore_object("/1.o");
  tell_object(receiver, sprintf("3: %O\n", res));
}

int main(string arg)
{
  receiver = this_player();
  write("]");
  input_to( (: on_input :), 2);
  return 1;
}
