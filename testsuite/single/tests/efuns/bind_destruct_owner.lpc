// bind() runs the valid_bind() master apply -- arbitrary LPC -- and then used
// to store the new owner into the funptr from a raw pointer captured BEFORE
// that apply. If valid_bind() destructed the new owner, the destruct sweep
// (remove_object_from_stack) had already released the VM stack's reference,
// so the funptr held an uncounted owner reference; its eventual dealloc
// over-decremented a reference belonging to someone else (issue #1327's
// corruption class). bind() must instead error out cleanly.

object target;

int valid_bind_hook(object binder, object old_owner, object new_owner) {
  if (target && new_owner == target) {
    destruct(target);
  }
  return 1;
}

void do_tests() {
  mixed err;

  target = clone_object("/clone/testob1");
  "/single/master"->set_bind_hook(this_object());
  err = catch(bind((: $1 + $2 :), target));
  "/single/master"->set_bind_hook(0);

  // The unfixed driver completed the bind against the destructed owner (and
  // silently corrupted its refcount); the fix reports a clean error.
  ASSERT(err);

  target = 0;
}
