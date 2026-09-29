// allocate_mapping(keys, callback) with a DUPLICATE key re-finds the existing
// node, and find_for_insert() frees the old value before the callback runs.
// The freed-but-still-populated slot then sat in live mapping data while the
// callback executed: a callback that error()s made the unwind free the
// half-built mapping and release that value a SECOND time -- an object-ref
// over-decrement of issue #1327's class (on a Debug build the fatal
// "ref count 0, but not destructed" fires right here). find_for_insert() now
// zeroes the slot after freeing it.

object ob;
int calls;

mixed cb(mixed key) {
  if (calls++) {
    error("boom");
  }
  return ob;
}

void do_tests() {
  mixed err;

  calls = 0;
  ob = clone_object("/clone/testob1");

  err = catch(allocate_mapping(({ "dup", "dup" }), (: cb :)));
  ASSERT(err);

  // The clone must still be alive with a sane refcount: releasing our
  // reference must not be a second deallocation.
  ASSERT(objectp(ob));
  destruct(ob);
  ob = 0;
}
