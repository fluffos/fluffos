void create(int arg) {
#ifndef __NO_SHADOWS__
    if (arg) {
	ASSERT(shadow(previous_object()));
	ASSERT(catch(shadow(previous_object())));
    }
#endif
}

object foo() {
    return this_object();
}
