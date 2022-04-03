string result = "";

void on_input_long(string item) {
  write(item);

  if(item == ".") {
    // get rid of last newline
    result = result[0..<2];
    write("\nReceived: \n" + result + "\n");
    return ;
  }
  result += item + "\n";

  input_to("on_input_long", 1);
}

void on_input_secure(string item) {
  write("received: " + item + "\n");
}

void on_input_1(string item) {
  write("received: " + item + "\n");
}

int main(string arg) {
  if(arg == "1") {
    write("prompt]");
    input_to( (: on_input_1 :), 0);
  } else if (arg == "2"){
    write("secure prompt]");
    input_to( (: on_input_secure :), 1);
  } else if (arg == "3"){
    write("long prompt]");
    result = "";
    input_to( (: on_input_long :), 1);
  }
  return 1;
}
