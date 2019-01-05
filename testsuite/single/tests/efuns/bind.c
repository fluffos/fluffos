int glob;

#define CANT_BIND(f) ASSERT(catch(bind(f, previous_object())))
#define CAN_BIND(f) ASSERT(bind(f, previous_object()))

void do_tests() {
    CANT_BIND((: do_tests :));
    CANT_BIND((: glob :));
    CANT_BIND((: glob + 1 :));
    CANT_BIND((: do_tests() :));
    CAN_BIND((: $1 + $2 :));
    ASSERT(functionp(bind( (: $1 :), this_object())));
    ASSERT(evaluate(bind( (: $1 + $2 :), this_object()), 3, 4) == 7);
}
