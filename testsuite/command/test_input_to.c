string result = "";

void on_input_5(string item) {
  if(item == ".") {
    // get rid of last newline
    result = result[0..<2];
    write("\nReceived: \n" + result + "\n");
    return ;
  }
  result += item;
  result += "\n";

  input_to("on_input_5");
}

void on_input_4(string item) {
  write("received: " + item + "\n");

  write("long edit]\n");
  result = "";
  input_to( (: on_input_5 :) );
}
void on_input_3(string item) {
  write("received: " + item + "\n");

  write("secure char prompt]");
  input_to( (: on_input_4 :), 3);
}
void on_input_2(string item) {
  write("received: " + item + "\n");

  write("char prompt]");
  input_to( (: on_input_3 :), 2);
}

void on_input_1(string item) {
  write("received: " + item + "\n");

  write("secure prompt]");
  input_to( (: on_input_2 :), 1);
}

int main(string arg) {
  write("prompt]");
  input_to( (: on_input_1 :), 0);
  return 1;
}
