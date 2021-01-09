#define OTHER_TESTS
#define STRING_TESTS
#define ARRAY_TESTS
#define MAPPING_TESTS

inherit "/single/inh";

int time;
int save;
mixed * a_global_var;
mapping another;
string one_more;

#define out(x) write(x)

create() {
    a_global_var = ({ 1, 3, 5, ({ 4, 1 }) });
    another = ([ 1 : "foo", 3 : "bar" ]);
    one_more = "Hmm";
}

void lfun0() {
}

void lfun1(mixed a1) {
}

void lfun2(mixed a1, mixed a2) {
}

void lfun3(mixed a1, mixed a2, mixed a3) {
}

int* _powLF(int n) {
  int* res;
  int L, F;

  if (n == 1) {
    return ({1 , 1});
  }

  res = _powLF(n / 2);
  L = res[0];
  F = res[1];
  res = ({ (L * L + 5 * F * F) >> 1, L * F });
  L = res[0];
  F = res[1];

  if (n & 1) {
    return ({ (L + 5 * F) >> 1, (L + F) >> 1 });
  } else {
    return res;
  }
}

int fib(int n) {
  if (n & 1) {
    return _powLF(n)[1];
  } else {
    int *res = _powLF(n / 2);
    return res[0] * res[1];
  }
}

int fib_recur(int n) {
  if(n == 0){
    return 0;
  } else if(n == 1) {
    return 1;
  } else {
    return (fib_recur(n-1) + fib_recur(n-2));
  }
}

#define START do { reset_eval_cost();  set_eval_limit(0x7fffffff);  before = perf_counter_ns(); } while (0)
#define END   after = perf_counter_ns(); time = (after - before);

#define REPORT(z, t) out(sprintf("%-30s: %10d ns\n", z, t))
#define LOOP(n, x) for (i = 0; i < (n); i++) { x; }

#define DIFF(z, n, x, y) START; LOOP(n, x); END; save = time; \
                         START; LOOP(n, y); END; REPORT(z, time - save)
#define SINGLE(z, x) START; x; END; REPORT(z, time)

/* Evidentally, 3.2 doesn't have token pasting */
#define INIT(n) START; LOOP(n, ); END; empty##n = time

#define TIMEDIFF(z, n, x, d) START; LOOP(n, x); END; REPORT(z, ((time - d) / n))
#define TIME(z, n, x) START; LOOP(n, x); END; REPORT(z, ((time) / n))
#define SAVETIME(z, n, x) START; LOOP(n, x); END; z = time

int main() {
    int before, after;
    int i;
#ifdef STRING_TESTS
    string s, s1, s2, s3;
#endif
#ifdef ARRAY_TESTS
    mixed *a, *a1, *a2, *a3;
#endif
#ifdef MAPPING_TESTS
    mapping m, m1, m2;
#endif
    int empty300, empty1000, empty10000, empty20000, empty50000,
        empty100000, empty200000, empty1000000;
    int save;
    int ss, sm;

#ifdef STRING_TESTS
    s1 = "This is a test";
    s2 = " of the emergency broadcast system.";
    s3 = s1 + s2;
#endif
#ifdef ARRAY_TESTS
    a1 = ({ 5, 1, 3, 4, 2 });
    a2 = ({ 9, 8, 7, 6, 10 });
    a3 = ({ 4, 1 });
#endif
#ifdef MAPPING_TESTS
    m1 = ([ "1" : "a", "2" : "b", "3" : "c", "4" : "d", "5" : "e" ]);
    m2 = ([ 1 : "a", 2 : "b", 3 : "c", 4 : "d", 5 : "e" ]);
#endif

    write(__VERSION__+"\n");

    // traditional recursive fib : ~3x compare to python3
    ASSERT_EQ(55, fib_recur(10));
    ASSERT_EQ(6765, fib_recur(20));
    TIME("fib_recur(10)",10000, fib_recur(10));
    TIME("fib_recur(20)",100, fib_recur(20));

    // more optimized algo: ~3x compare to python3 too
    ASSERT_EQ(55, fib(10));
    ASSERT_EQ(1836311903, fib(46));
    ASSERT_EQ(2971215073, fib(47));
    ASSERT_EQ(7540113804746346429, fib(92));
    TIME("fib(10)",10000, fib(10));
    TIME("fib(46)",10000, fib(46));
    TIME("fib(47)",10000, fib(47));
    TIME("fib(92)",10000, fib(92));

#ifdef OTHER_TESTS
    TIME("instruction", 10000000, 1);
    TIME("empty for", 100, for (i = 0; i < 10000; i++) ;);
    TIME("while", 10, {int x = 10000; while (x--);} );
    TIME("while2", 10, {int x = 10000; while (1) { if (!(x--)) break; }});
    SAVETIME(ss, 1000000, save = 0);
    SAVETIME(sm, 1000000, save = 1);
    save = 0;
    TIMEDIFF("if (x == 0) T",1000000,if (save == 0) { save = 0; }, ss);
    save = 1;
    TIME("if (x == 0) F",1000000,if (save == 0) { });
    save = 1;
    TIMEDIFF("if (x) T",1000000,if (save) { save = 1; }, sm);
    save = 0;
    TIME("if (x) F",1000000,if (save) { });
    save = 0;
    TIMEDIFF("if (x < n) T",1000000,if (save < 1) { save = 0; }, ss);
    save = 1;
    TIME("if (x < n) F",1000000,if (save < 1) { });
    save = 0;
    TIMEDIFF("if (!x) T",1000000,if (!save) { save = 0; }, ss);
    save = 1;
    TIME("if (!x) F",1000000,if (!save) { });
    save = 1;
    TIMEDIFF("if (x == n) T",1000000,if (save == 1) { save = 1; }, sm);
    save = 0;
    TIME("if (x == n) F",1000000,if (save == 1) { });
    SAVETIME(save, 1000000, sm = 0);
    sm = 0;
    ss = 1;
    TIME("if (x && y && z) F",1000000, if (sm && sm && sm) { sm = 0; });
    TIME("if (x && y && z) TF",1000000, if (ss && sm && sm) { sm = 0; });
    TIME("if (x && y && z) TTF",1000000, if (ss && ss && sm) { sm = 0; });
    TIMEDIFF("if (x && y && z) TTT",1000000,  if (ss && ss && ss) { sm = 0; }, save);
#endif
#ifdef STRING_TESTS
    TIME("string assign (s)",1000000, s = s1);
    TIME("string assign (m)",1000000, s = s3);
    TIME("string add (ss)",1000000, s1 + s2);
    TIME("string add (sm)",1000000,s1 + s3);
    TIME("string add (ms)",1000000,s3 + s1);
    TIME("string add (mm)",1000000,s3 + s3);
    SAVETIME(ss, 100000, s = s1);
    SAVETIME(sm, 100000, s = s3);
    TIMEDIFF("string += (ss)",100000,s = s1; s += s2, ss);
    TIMEDIFF("string += (sm)",100000,s = s1; s += s3, ss);
    TIMEDIFF("string += (ms)",100000,s = s3; s += s1, sm);
    TIMEDIFF("string += (mm)",100000,s = s3; s += s3, sm);
#endif
#ifdef ARRAY_TESTS
    TIME("allocate array",10000, a = allocate(100));
    TIME("array creation (int)",10000, ({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }));
    TIME("array creation (string)",10000,({ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }));
    TIME("array assign",10000,a = a1);
    TIME("array addition",10000,a1 + a2);
    TIME("array subtraction",10000,a1 - a3);
    TIME("array lookup",10000,a1[3]);
    SAVETIME(save,10000,a = a1);
    TIMEDIFF("array +=",10000,a = a1; a += a2,save);
    TIMEDIFF("array -=",10000,a = a1; a -= a3, save);
#endif
#ifdef MAPPING_TESTS
    TIME("allocate mapping",10000, m = allocate_mapping(100));
    TIME("mapping creation (int)",10000, ([ 1 : 2, 3 : 4, 5 : 6, 7 : 8 ]));
    TIME("mapping creation (string)",10000, ([ "1" : "a", "2" : "b", "3" : "c", "4" : "d", "5" : "e" ]));
    TIME("mapping assign",10000, m = m1);
    TIME("lookup string (exist)",10000, m1["3"]);
    TIME("lookup string (missing)",10000, m1["6"]);
    TIME("lookup int (exist)",10000, m2[3]);
    TIME("lookup int (missing)",10000, m2[10]);
    SAVETIME(save,100000, m = m1);
    TIMEDIFF("mapping insert",100000, m = m1; m["6"] = "f", save);
    TIMEDIFF("mapping replace",100000, m = m1; m["2"] = 0, save);
    TIMEDIFF("mapping delete",100000, m = m1; map_delete(m, "2"), save);
#endif
#ifdef OTHER_TESTS
    TIME("local call (no args)",100000, lfun0());
    TIME("local call (1 arg)",100000, lfun1(1));
    TIME("local call (2 args)",100000, lfun2("foo", 1));
    TIME("local call (3 args)",100000, lfun3("foo", 1, i));
    TIME("simul_efun",100000, simul());
    TIME("call_other",100000,this_object()->lfun0());
    TIME("call_other (string)",100000,"/command/speed"->lfun0());
    TIME("call_other (miss)",100000,this_object()->doesnt_exist());
    TIME("inherited call",100000,ifun());
    TIME("explicit inherited",100000,inh::ifun());
    TIME("save_object",300,save_object("/tmp"));
    TIME("restore_object",1000,restore_object("/tmp"));
#endif
    return 1;
}
