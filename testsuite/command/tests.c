#include <globals.h>

int execute(string fun);

void recurse(string dir) {
  mixed leaks;

  foreach (string file in sort_array(get_dir(dir + "*.c"), (: random(2) - random(2) :))) {
    execute(dir + file);
  }
  foreach (string subdir in map(filter(get_dir(dir + "*", -1),
          (: $1[1] == -2 :)),
        (: $1[0] :)) - ({ ".", ".." }))
  {
    if (subdir == "fail") {
      foreach (string fn in get_dir(dir + "fail/*.c")) {
        write("A> " + dir + "fail/" + fn + "\n");
        ASSERT2(catch(load_object(dir+"fail/"+fn)), "fail/" + fn + " loaded");
#if defined(__DEBUGMALLOC__) && defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__PACKAGE_DEVELOP__)
        leaks = check_memory();
        if (sizeof(filter(explode(leaks, "\n"), (: $1 && $1[0] :))) != 1) {
          write("After trying to compile: " + dir + "fail/" + fn + "\n");
          write(leaks);
          error("LEAK\n");
        }
#endif
      }
      cp("/log/compile", "/log/compile_fail");
      rm("/log/compile");
    }
    // only make sure crasher don't crash, errors are ignored.
    else if (subdir == "crasher") {
      foreach (string fn in get_dir(dir + subdir + "/*.c")) {
        write("B> " + dir + subdir + "/" + fn + "\n");
        catch((dir + subdir + "/" + fn + ".c")->do_tests());
#if defined(__DEBUGMALLOC__) && defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__PACKAGE_DEVELOP__)
        leaks = check_memory();
        if (sizeof(filter(explode(leaks, "\n"), (: $1 && $1[0] :))) != 1) {
          write("After trying to run: " + dir + subdir + "/" + fn + ".c\n");
          write(leaks);
          error("LEAK\n");
        }
#endif
      }
    }
    else {
      recurse(dir + subdir + "/");
    }
  }
}
int execute(string fun)
{
  string leaks;
  object tp = this_player();

  if (!fun || fun == "") {
    recurse("/single/tests/");
    write("Checks succeeded.\n");
    return 1;
  }

  set_eval_limit(0x7fffffff);

  write("C> " + fun + "\n");
  ASSERT_EQ(0, catch(fun->do_tests()));

  set_eval_limit(0x7fffffff);

  if (tp != this_player())
    error("Bad this_player() after calling " + fun + "\n");
#if defined(__DEBUGMALLOC__) && defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__PACKAGE_DEVELOP__)
  leaks = check_memory();
  if (sizeof(filter(explode(leaks, "\n"), (: $1 && $1[0] :))) != 1) {
    write("After calling: " + fun + "\n");
    write(leaks);
    error("LEAK\n");
  }
#endif
  return 1;
}

int main() {
#if !(defined(__DEBUGMALLOC__) && defined(__DEBUGMALLOC_EXTENSIONS__) && defined(__PACKAGE_DEVELOP__))
  write("WARNING: Possible RELEASE build, check_memory() is not being executed.\n");
#endif
  return execute("");
}
