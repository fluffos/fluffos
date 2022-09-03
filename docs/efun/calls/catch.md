---
layout: default
title: calls / catch
---

### NAME

    catch() - catch an evaluation error

### SYNOPSIS

    mixed catch( mixed expr );

### DESCRIPTION

    Evaluate  <expr>.  If  there  is no error, 0 is returned. If there is a
    standard error, a string (with a leading '*') will be returned.

    The function throw() can also be used to immediately return any  value,
    except 0. catch() is not really a function call, but a directive to the
    compiler.

    The catch() is somewhat costly, and should not be used  just  anywhere.
    Rather, use it at places where an error would destroy consistency.

### EXAMPLE

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

### SEE ALSO

    error(3), throw(3), error_handler(4)

