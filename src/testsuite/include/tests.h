#ifndef TESTS_H
#define TESTS_H

#define ASSERT(x) if (!(x)) { error(file_name() + ": Check failed.\n"); }
#define ASSERT2(x, r) if (!(x)) { error(file_name() + ":" + r + ", Check failed.\n"); }

#define SAVETP tp = this_player()
#define RESTORETP { if (tp) evaluate(bind( (: enable_commands :), tp)); else { object youd_never_use_this_as_a_var = new("/single/void"); evaluate(bind( (: enable_commands :), youd_never_use_this_as_a_var)); destruct(youd_never_use_this_as_a_var); } }

#ifdef __OLD_TYPE_BEHAVIOR__
#define TYPETEST scream and die
#else
#define TYPETEST
#endif

#endif
