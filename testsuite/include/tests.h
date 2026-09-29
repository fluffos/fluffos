#ifndef TESTS_H
#define TESTS_H

#define SAFE(x) do {x} while(0)

#define CLEAR_ERROR (("/single/master"->clear_last_error() || 1))

// A failed check is written immediately (with the caught error trace),
// RECORDED with the master, and the run continues -- gtest semantics.
// The runner (/command/tests.lpc) reports every recorded failure at the
// end and exits nonzero; a stray failure outside a runner still fails
// the run through master::flag()'s recap.
#define OUTPUT(x) SAFE(write(catch(error(x))); \
  "/single/master"->record_failure(x);)
#define WHERE __FILE__ + ":" + __LINE__

#define ASSERT(x) if (CLEAR_ERROR && !(x)) { OUTPUT(WHERE + ", Check failed.\n"); }
#define ASSERT2(x, r) if (CLEAR_ERROR && !(x)) { OUTPUT(WHERE + ", Check failed: " + r + ".\n"); }
#define ASSERT_EQ(x, y) do { CLEAR_ERROR; __assert_eq((x), (y), WHERE); } while (0)
#define ASSERT_NE(x, y) do { CLEAR_ERROR; __assert_ne((x), (y), WHERE); } while (0)

// Exit latches (see /single/master.lpc). Declare work that outlives
// do_tests() -- a call_out, a parked `await`, package I/O -- so the runner
// holds the end of the run open until it finishes and its assertions are
// COUNTED instead of landing after the recap:
//
//   void do_tests() {
//     EXPECT_LATCH("timer");
//     call_out(function() { ASSERT_EQ(0, v); RELEASE_LATCH("timer"); }, 1);
//   }
//
// A latch that is never released fails the run naming the test and the
// latch, so "the deferred half never ran" can no longer pass silently.
#define EXPECT_LATCH(name) "/single/master"->expect_latch(name)
#define RELEASE_LATCH(name) "/single/master"->release_latch(name)

#define SAVETP tp = this_player()
#define RESTORETP { if (tp) evaluate(bind( (: enable_commands :), tp)); else { object youd_never_use_this_as_a_var = new("/single/void"); evaluate(bind( (: enable_commands :), youd_never_use_this_as_a_var)); destruct(youd_never_use_this_as_a_var); } }

#ifdef __OLD_TYPE_BEHAVIOR__
#define TYPETEST scream and die
#else
#define TYPETEST
#endif

#endif
