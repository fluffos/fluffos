---
title: calls / catch
---
# catch

### NAME

    catch - catch an evaluation error

### SYNOPSIS

    mixed catch( expr );
    mixed catch { statements }

### DESCRIPTION

    catch is a language construct (a compiler keyword), not an ordinary
    efun.  It accepts two body styles, sharing the same grammar as
    time_expression:

    - Parenthesized expression form: `catch(expr)` evaluates <expr>.
    - Block form: `catch { statements }` executes the statement block.

    If there is no error, 0 is returned.  If there is a standard error, a
    string (with a leading '*') will be returned.  The value of the body
    expression itself is discarded.

    The function throw() can also be used to immediately return any  value,
    except 0.

    It is a compile-time error to `break` or `continue` out of a catch
    block.

    The catch() is somewhat costly, and should not be used  just  anywhere.
    Rather, use it at places where an error would destroy consistency.

### EXAMPLE

    ```c
    void example1() {
        object ob ;
        mixed err ;

        err = catch( ob = load_object("/obj/weapon/sword") ) ;
        if(err) throw("There was an error loading the specified file.") ;
    }

    void example2() {
        mixed err = catch {
            string file, *files = ({
                "/u/g/gesslar/one",     // good file
                "/u/g/gesslar/two",     // bad file
                "/u/g/gesslar/three",   // good file
            }) ;

            foreach(file in files) load_object(file) ;
        } ;
        
        if(err) printf("ERR: %O", err) ;
    }

    // ERR: "*Error in loading object '/u/g/gesslar/two'"

    // A caught value is not always an error message.  throw() hands back
    // whatever it was given, so a class makes a tidy typed failure that
    // the catching code can inspect field by field instead of parsing a
    // string.
    class failure {
        string kind ;
        mixed detail ;
    }

    private void charge(object who, int amount) {
        int have = who->query_coins() ;

        if(have < amount)
            throw(new(class failure,
                      kind: "insufficient_funds",
                      detail: amount - have)) ;

        who->add_coins(-amount) ;
    }

    void example3(object who, int price) {
        mixed err = catch( charge(who, price) ) ;

        // classp() must be tested first: err is just as likely to be a
        // driver error string, and the && stops there when it is.
        if(classp(err) && err.kind == "insufficient_funds")
            write("You are " + err.detail + " coins short.\n") ;
        else if(err)
            throw(err) ;    // not ours to handle -- pass it along
    }
    ```

### SEE ALSO

    error(3), throw(3), error_handler(4), time_expression(3)

