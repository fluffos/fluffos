mapping fd_to_msg;
mapping fd_to_obj;

void on_read(int fd, string msg) {
  debug_message(sprintf("on_read: %d", fd));

  fd_to_msg[fd] += sprintf("%s", msg);
}

void on_write(int fd) {
  debug_message(sprintf("on_write: %d", fd));
}

void on_close(int fd) {
  debug_message(sprintf("on_close: %d", fd));

  tell_object(fd_to_obj[fd], fd_to_msg[fd]);
  map_delete(fd_to_msg, fd);
  map_delete(fd_to_obj, fd);

  // Turn off debug messages for external_start
  clear_debug_level("external_start") ;
}

int main(string arg)
{
  int fd;
  int CURL_CMD = 1;
  if(__ARCH__ == "Microsoft Windows") CURL_CMD = 2;

  if (!arg) arg = "";

  // Turn on debug messages for external_start
  set_debug_level("external_start") ;
  fd = external_start(CURL_CMD, arg, "on_read", "on_write", "on_close");

  if (!fd_to_msg) {
    fd_to_msg = ([]);
  }
  if (!fd_to_obj) {
    fd_to_obj = ([]);
  }


  fd_to_msg[fd] = "";
  fd_to_obj[fd] = this_player();

  return 1;
}
