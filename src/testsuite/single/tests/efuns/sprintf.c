// need to add:
// = and # mode tests
// precision
// : and @
// 'X'

// specifier, result, args
mixed make_arg(int x, int trail) {
#define BEFORE_INTS 0
#define INTS_HI 2
#define INTS_LO 6
#define BEFORE_STRS (INTS_HI * INTS_LO)
#define STRS_HI 1
#define STRS_LO 1
#define BEFORE_FLOS (BEFORE_STRS + STRS_HI * STRS_LO)
#define FLOS_HI 1
#define FLOS_LO 1
#define BEFORE_CHRS (BEFORE_FLOS + FLOS_HI * FLOS_LO)
#define CHRS_HI 1
#define CHRS_LO 1
#define BEFORE_LPCS (BEFORE_CHRS + CHRS_HI * CHRS_LO)
#define LPCS_HI 1
#define LPCS_LO 9
#define TOTAL (BEFORE_LPCS + LPCS_HI * LPCS_LO)

    mixed ret;
    string lpad = " ", rpad = " ", mpad;
    int tmp;


    switch (x % TOTAL) {
    case BEFORE_INTS..BEFORE_INTS + INTS_HI * INTS_LO - 1:
	switch (x % INTS_LO) {
	case 0: ret = ({ "i", "5", 5 }); break;
	case 1: ret = ({ "d", "345", 345 }); break;
	case 2: ret = ({ "o", "111", 73 }); break;
	case 3: ret = ({ "x", "a9", 169 }); break;
	case 4: ret = ({ "X", "A9", 169 }); break;
	case 5: ret = ({ "i", "-345", -345 }); break;
	}
	switch (x / INTS_LO) {
	case 0:
	    break;
	case 1:
	    if (ret[0][0] != 'i' && ret[0][0] != 'd')
		break;
	    ret[0] = "+" + ret[0];
	    if (ret[1][0] != '-') ret[1] = "+" + ret[1];
	    lpad = "+";
	    break;
	}
	break;
    case BEFORE_STRS..BEFORE_STRS + STRS_HI * STRS_LO - 1:
	x -= BEFORE_STRS;
	switch (x % STRS_LO) {
	case 0: ret = ({ "s", "foo", "foo" }); break;
	}
	switch (x / STRS_LO) {
	case 0:
	    break;
	}
	break;
    case BEFORE_FLOS..BEFORE_FLOS + FLOS_HI * FLOS_LO - 1:
	x -= BEFORE_FLOS;
	switch (x % FLOS_LO) {
	case 0: ret = ({ "f", "1.234000", 1.234 }); break;
	}
	switch (x / FLOS_LO) {
	case 0:
	    break;
	}
	break;
    case BEFORE_CHRS..BEFORE_CHRS + CHRS_HI * CHRS_LO - 1:
	x -= BEFORE_CHRS;
	switch (x % CHRS_LO) {
	case 0: ret = ({ "c", "a", 97 }); break;
	}
	switch (x / CHRS_LO) {
	case 0:
	    break;
	}
	break;
    case BEFORE_LPCS..BEFORE_LPCS + LPCS_HI * LPCS_LO - 1:
	x -= BEFORE_LPCS;
	switch (x % LPCS_LO) {
	case 0: ret = ({ "O", "1", 1 }); break;
	case 1: ret = ({ "O", "1.500000", 1.5 }); break;
	case 2: ret = ({ "O", "\"foo\"", "foo" }); break;
	case 3: ret = ({ "O", "({ })", ({}) }); break;
	case 4: ret = ({ "O", "({ /* sizeof() == 2 */\n  1,\n  2\n})", ({ 1, 2 }) }); break;
	case 5: ret = ({ "O", "(: make_arg, 5 :)", (: make_arg, 5 :) }); break;
	case 6: ret = ({ "O", "(: <code>($1) :)", (: $1 :) }); break;
	case 7: ret = ({ "O", "(: sprintf :)", (: sprintf :) }); break;
	case 8: ret = ({ "O", "([ /* sizeof() == 1 */\n  1 : 2,\n])", ([ 1 : 2 ]) }); break;
	}
	switch (x / LPCS_LO) {
	case 0:
	    break;
	}
    }

    tmp = random(20);
    if (random(5)) {
	mpad = "xyzzy";
	switch (x / TOTAL) {
	case 0:
	    break;
	case 1:
	    ret[0] = "'xyzzy'" + tmp + ret[0];
	    if (strlen(ret[1]) < tmp) {
		ret[1] = mpad[0..((tmp - strlen(ret[1])) % strlen(mpad))-1]
		    + ret[1];
		while (strlen(ret[1]) + strlen(mpad) <= tmp)
		    ret[1] = mpad + ret[1];
	    }
	    break;
	case 2:
	    {
		int n, ll, rr;
		ret[0] = "|'xyzzy'" + tmp + ret[0];
		if ((n = strlen(ret[1])) < tmp) {
		    ll = (tmp - n + 1)/2;
		    if (ll % strlen(mpad))
			ret[1] = mpad[0..(ll % strlen(mpad))-1] + ret[1];
		    ll = ll/strlen(mpad);
		    while (ll--)
			ret[1] = mpad + ret[1];
		    if (trail) {
			rr = (tmp - n)/2;
			n = (rr % strlen(mpad));
			rr = rr/strlen(mpad);
			while (rr--)
			    ret[1] += mpad;
			if (n)
			    ret[1] += mpad[0..n-1];
		    }
		}
	    }
	    break;
	case 3:
	    ret[0] = "-'xyzzy'" + tmp + ret[0];
	    if (strlen(ret[1]) < tmp) {
		int n = ((tmp - strlen(ret[1])) % strlen(mpad));
		while (strlen(ret[1]) + strlen(mpad) <= tmp)
		    ret[1] += mpad;
		if (n)
		    ret[1] += mpad[0..n-1];
	    }
	    break;
	case 4:
	    ret[0] = "-*" + tmp + ret[0];
	    ret = ret[0..1] + ({ tmp }) + ret[2..];
	    while (strlen(ret[1]) < tmp)
		ret[1] += rpad;
	    break;
	default:
	    return 0;
	}
    } else
	switch (x / TOTAL) {
	case 0:
	    break;
	case 1:
	    ret[0] = tmp + ret[0];
	    while (strlen(ret[1]) < tmp)
		ret[1] = lpad + ret[1];
	    break;
	case 2:
	    {
		int n, ll, rr;
		ret[0] = "|" + tmp + ret[0];
		if ((n = strlen(ret[1])) < tmp) {
		    ll = (tmp - n + 1)/2;
		    rr = (tmp - n)/2;
		    while (ll--)
			ret[1] = lpad + ret[1];
		    if (trail)
			while (rr--)
			    ret[1] += rpad;
		}
	    }
	    break;
	case 3:
	    ret[0] = "-" + tmp + ret[0];
	    while (strlen(ret[1]) < tmp)
		ret[1] += lpad;
	    break;
	case 4:
	    ret[0] = "-*" + tmp + ret[0];
	    ret = ret[0..1] + ({ tmp }) + ret[2..];
	    while (strlen(ret[1]) < tmp)
		ret[1] += lpad;
	    break;
	default:
	    return 0;
	}
    ret[0] = "%" + ret[0];
    return ret;
}

void do_tests() {
    int x;
    mixed *mx;
    string format, shouldbe;
    string hmm;
    
    while (1) {
	if (!(mx = make_arg(x++, 1))) break;
	
	format = "$" + mx[0] + "^";
	shouldbe = "$" + mx[1] + "^";
	ASSERT2(sprintf(format, mx[2..]...) == shouldbe, shouldbe);
    }

    hmm = sprintf("xxx%#-20.5s\n", "This\nis\na\ntest\nof\nsprintf\ntable\nmode\n");
    ASSERT2(hmm == "xxxThisa   of  tabl\n   is  testsprimode\n", hmm);
    hmm = sprintf("xxx%#-'y'20.5s\n", "This\nis\na\ntest\nof\nsprintf\ntable\nmode\n");
    ASSERT2(hmm == "xxxThisayyyofyytablyyyy\n   isyytestsprimodeyyyy\n", hmm);
    
    hmm = sprintf("%#-20s\n", "a\nb\nc\nd\ne\nf\ng\nh\ni\nj\nk\nl\no\n");
    ASSERT2(hmm == "a   d   g   j   o\nb   e   h   k   \nc   f   i   l   \n", hmm);
}
