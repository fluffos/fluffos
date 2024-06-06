---
layout: doc
title: apply / interactive / receive_message
---
# receive_message

### NAME

    receive_message - provides the interface used by the message efun.

### SYNOPSIS

    void receive_message( string type, string message );

### DESCRIPTION

    The  message() efun calls this method in the living targets' objects. The 
    type parameter is typically used to indicate the type (say, tell, emote,
    combat, room description, etc) of the message. The receive_message()
    apply together with the message() efun can provide a good mechanism for
    interfacing with a "smart" client.

### EXAMPLE

```c
void receive_message(string type, string message) {
    if(member_array(type, arrayOfIgnoredMessageTypes) != -1) {
        // simple filtering example
        return;
    } else if(type == "say") {
       // simple formatting example
        message = "%^CYAN%^" + replace_string(message, ":", ":%^RESET%^");
    }
    receive(message);
}
```
### SEE ALSO

    catch_tell(4), message(3), receive(3), receive_snoop(4)

