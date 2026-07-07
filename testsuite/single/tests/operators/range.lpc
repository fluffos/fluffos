#define ARR ({ 1, 2, 3 })

void do_tests() {
    mixed arr = ({ 1, 2, 3 });
#ifndef __OLD_RANGE_BEHAVIOR__
    ASSERT(arr[0..-1] == ({}));
    ASSERT(ARR[0..-1] == ({}));
    ASSERT(same(arr[0..<1], ({1, 2, 3})));
    ASSERT(same(ARR[0..<1], ({1, 2, 3})));
    ASSERT(same(arr[1..<1], ({2, 3})));
    ASSERT(same(ARR[1..<1], ({2, 3})));
    ASSERT(same(arr[0..<2], ({1, 2})));
    ASSERT(same(ARR[0..<2], ({1, 2})));
    ASSERT(same(arr[1..1], ({ 2 })));
    ASSERT(same(ARR[1..1], ({ 2 })));
#endif
}
