#pragma no_strict_types
foo() { }
bar(x) { }
#pragma strict_types
create() { }

void foo(int);

; // only a warning

int ident;
#define X1 id
#define X2 ent
int paste = X1##X2;
string x = @END
xxx
END;
string * y = @@END
xxx
yyy
zzz
END
;

