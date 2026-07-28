---
title: calls / throw
---
# throw

### NAME

    throw() - hand a value to the nearest enclosing catch()

### SYNOPSIS

    void throw(mixed value);

### DESCRIPTION

    Stops what is currently running and hands `value` to the nearest
    catch(3) above the current call.  That catch() returns `value` instead
    of its usual 0.

    throw() never comes back, so anything written after it in the same
    function will never run.

    Any value can be thrown, and catch() gives it back exactly as it was
    thrown -- a string, an array, a mapping, a class, an object.  Nothing
    is converted to text and nothing is added to it.  That makes throw()
    useful for more than error messages: you can hand a whole structured
    result back to code that is prepared to deal with it.

    Throwing 0 cannot be detected.  catch() already returns 0 to mean
    "nothing went wrong", so throw(0) looks exactly like the body finishing
    normally.  Throw anything else.

    Only the innermost catch() sees the value.  It does not keep travelling
    outward on its own -- if the code that caught it wants to pass it
    along, it has to throw() it again.

### HOW FAR IT TRAVELS

    A thrown value travels up through as many ordinary calls as it needs
    to.  It does not matter how deep you are, and it does not matter how
    you got there: calls inside the same object, calls through
    `ob->func()`, inherited functions, function pointers invoked with
    evaluate(), and callbacks run by efuns such as filter() and
    sort_array() all pass a thrown value straight through to the catch()
    above them.  Even a create() that throws while the object is being
    loaded inside `catch(load_object(...))` is caught normally.

    What a thrown value cannot do is escape a call that the driver started
    on its own.  A call_out(), an input_to() callback, and applies the
    driver invokes on your object -- logon(), net_dead(),
    process_input(), reset(), clean_up(), on_destruct(), the telnet and
    GMCP applies -- each begin a fresh chain of calls with no catch()
    above them.  That holds even if the code that scheduled the call_out()
    was itself sitting inside a catch(): by the time the callback runs,
    that catch() has long since returned.  A throw() in one of those
    places is an error (see ERRORS below).  When a callback needs to
    handle a failure, put the catch() inside the callback.

### A THROWN VALUE IS NOT A DRIVER ERROR

    A value that arrives at a catch() by way of throw() has not been
    through the driver's error machinery at all.  No traceback is printed,
    nothing is written to the debug log, and the error_handler() apply in
    the master object is never called.

    A thrown string also does not get the leading '*' that the driver puts
    on its own error messages, and that '*' is how handling code tells the
    two apart:

    - `"*Bad argument 1 to move()\n"` came from the driver.
    - `"took too long\n"` came from a throw() somewhere in the mudlib.

    Keep that in mind when building a new message out of a caught one.
    Writing `throw("context: " + err)` when `err` came from the driver
    buries the '*' in the middle of the new string, and anything checking
    `err[0] == '*'` will stop recognising it as a driver error.

### ERRORS

    A throw() with no catch() above it in the current chain of calls
    raises a real runtime error, `"*Throw with no catch.\n"` -- traceback,
    error handler and all -- and the value you threw is discarded.

### EXAMPLE

```c
// Passing a failure further up, with more context added.
void move_or_fail(object ob, object dest) {
    mixed err = catch(ob->move(dest));

    if (err) {
        // Only a driver error carries the leading '*', so only put one
        // back when it was there to begin with -- adding it to a mudlib
        // value would disguise it as a driver error.
        if (stringp(err) && err[0] == '*')
            throw("*move_or_fail(): " + err[1..]);
        else if (stringp(err))
            throw("move_or_fail(): " + err);
        else
            throw(err);     // not a string -- pass it through untouched
    }
}
```

```c
// Throwing a value rather than a message, so the caller can act on the
// details instead of picking a string apart.
private void charge(object who, int amount) {
    int have = who->query_coins();

    if (have < amount)
        throw(({ "insufficient_funds", amount - have }));

    who->add_coins(-amount);
}

void buy(object who, int price) {
    mixed err = catch(charge(who, price));

    if (arrayp(err) && err[0] == "insufficient_funds")
        write("You are " + err[1] + " coins short.\n");
    else if (err)
        throw(err);     // not ours to handle -- pass it along
}
```

### SEE ALSO

    catch(3), error(3), error_handler(4)
