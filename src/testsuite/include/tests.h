#ifndef TESTS_H
#define TESTS_H

#define SAFE(x) do {x} while(0)

#define OUTPUT(x) SAFE(write(catch(error(x))); \
  if(!this_player()) { shutdown(-1); })
#define WHERE __FILE__ + ":" + __LINE__

#define ASSERT(x) if (!(x)) { OUTPUT(WHERE + ", Check failed.\n"); }
#define ASSERT2(x, r) if (!(x)) { OUTPUT(WHERE + ", Check failed: " + r + ".\n"); }
#define ASSERT_EQ(x, y) if ((x) != (y) ) { \
  OUTPUT(WHERE + ", Check Failed: \n" + \
  "Expected: " + (x) + "Actual: " + (y) + "\n"); }

#define SAVETP tp = this_player()
#define RESTORETP { if (tp) evaluate(bind( (: enable_commands :), tp)); else { object youd_never_use_this_as_a_var = new("/single/void"); evaluate(bind( (: enable_commands :), youd_never_use_this_as_a_var)); destruct(youd_never_use_this_as_a_var); } }

#ifdef __OLD_TYPE_BEHAVIOR__
#define TYPETEST scream and die
#else
#define TYPETEST
#endif

#endif
