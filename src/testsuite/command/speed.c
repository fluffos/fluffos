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

#ifndef MUDOS
#define map_delete m_delete
#endif

#ifdef __DGD__
#define out(x) send_message(x)
#else
#define out(x) write(x)
#endif

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
#ifdef MUDOS
#define START do { reset_eval_cost();  set_eval_limit(0x7fffffff);  before = rusage(); } while (0)
#define END   after = rusage(); time = after["stime"] + after["utime"] - before["stime"] - before["utime"]
#else
#ifdef ANCIENT
#define START sscanf(rusage(), before, i); before += i
#define END   sscanf(rusage(), after, i); time = after + i - before
#else
#define START before = rusage()
#define END   after = rusage(); time = after[0] + after[1] - before[0] - before[1]
#endif
#endif

#define REPORT(z, t) out((z) + ": " + (t) + "\n")
#define LOOP(n, x) for (i = 0; i < (n); i++) { x; }

#define DIFF(z, n, x, y) START; LOOP(n, x); END; save = time; \
                         START; LOOP(n, y); END; REPORT(z, time - save)
#define SINGLE(z, x) START; x; END; REPORT(z, time)

/* Evidentally, 3.2 doesn't have token pasting */
#define INIT(n) START; LOOP(n, ); END; empty##n = time

#define TIMEDIFF(z, n, x, d) START; LOOP(n, x); END; REPORT(z, time - d)
#define TIME(z, n, x) START; LOOP(n, x); END; REPORT(z, time - empty##n)
#define SAVETIME(z, n, x) START; LOOP(n, x); END; z = time

int main() {
#ifdef MUDOS
    mapping before, after;
#else
    int *before, *after;
#endif
    int i;
#ifdef STRING_TESTS
    string s, s1, s2, s3;
#endif
#ifdef ARRAY_TESTS
    mixed *a, *a1, *a2, *a3;
#endif
#ifdef MAPPING_TESTS
    mapping m, m1;
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
#endif

    out("Initializing variables ...\n");
    INIT(300);
    INIT(1000);
    INIT(10000);
    INIT(20000);
    INIT(50000);
    INIT(100000);
    INIT(200000);
    INIT(1000000);

#ifdef OTHER_TESTS
    DIFF("instruction", 10000000, save, -save);
    SINGLE("empty for", for (i = 0; i < 3500000; i++) ;);
    i = 5000000; SINGLE("while", while (i--); );
    i = 2500000; SINGLE("while2", while (1) { if (!(i--)) break; });
#endif
#ifdef STRING_TESTS
    TIME("string assign (s)",1000000, s = s1);
    TIME("string assign (m)",200000, s = s3);
    TIME("string add (ss)",200000, s1 + s2);
    TIME("string add (sm)",200000,s1 + s3);
    TIME("string add (ms)",100000,s3 + s1);
    TIME("string add (mm)",100000,s3 + s3);
    SAVETIME(ss, 100000, s = s1);
    SAVETIME(sm, 100000, s = s3);
    TIMEDIFF("string += (ss)",100000,s = s1; s += s2, ss);
    TIMEDIFF("string += (sm)",100000,s = s1; s += s3, ss);
    TIMEDIFF("string += (ms)",100000,s = s3; s += s1, sm);
    TIMEDIFF("string += (mm)",100000,s = s3; s += s3, sm);
#endif
#ifdef ARRAY_TESTS
    TIME("allocate array",10000, a = allocate(100));
    TIME("array creation (int)",50000, ({ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 }));
    TIME("array creation (string)",20000,({ "1", "2", "3", "4", "5", "6", "7", "8", "9", "10" }));
    TIME("array assign",200000,a = a1);
    TIME("array addition",50000,a1 + a2);
    TIME("array subtraction",20000,a1 - a3);
    TIME("array lookup",100000,a1[3]);
    SAVETIME(save,50000,a = a1);
    TIMEDIFF("array +=",50000,a = a1; a += a2,save);
    TIMEDIFF("array -=",50000,a = a1; a -= a3, save);
#endif
#ifdef MAPPING_TESTS
    TIME("allocate mapping",50000, m = allocate_mapping(100));
    TIME("mapping creation (int)",50000, ([ 1 : 2, 3 : 4, 5 : 6, 7 : 8 ]));
    TIME("mapping creation (string)",50000, ([ "1" : "a", "2" : "b", "3" : "c", "4" : "d", "5" : "e" ]));
    TIME("lookup (exist)",100000, m1["3"]);
    TIME("lookup (missing)",100000, m1["6"]);
    TIME("mapping assign",200000, m = m1);
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
    TIME("call_other",50000,this_object()->lfun0());
#ifdef MUDOS
    TIME("call_other (string)",50000,"/command/speed"->lfun0());
#else
    TIME("call_other (string)",50000,"/command/speed"->lfun0());
#endif
    TIME("call_other (miss)",50000,this_object()->doesnt_exist());
    TIME("inherited call",100000,ifun());
    TIME("explicit inherited",100000,inh::ifun());
    TIME("save_object",300,save_object("/tmp/tmp"));
    TIME("restore_object",1000,restore_object("/tmp/tmp"));
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
    TIMEDIFF("if (x && y && z) TTT",1000000,  if (ss && ss && ss) { sm = 0; }, 
	 save);
#endif
    return 1;
}
