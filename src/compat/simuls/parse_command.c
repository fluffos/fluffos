private mixed *ret;
private mixed value;

private void
load_lpc_info(int ix, object ob)
{
    mixed *tmp, *sing;
    value ret;
    int il, make_plural = 0;
    string str;
    
    if (!ob)
	return;

    if (pluid_list &&
	sizeof(pluid_list) > ix &&
	pluid_list[ix] == 0) {
	ret = ob->parse_command_plural_id_list();
	if (arrayp(ret))
	    pluid_list[ix] = ret;
	else {
	    make_plural = 1;
	    pluid_list[ix] = 1;
	}
    }
    if (id_list &&
	sizeof(id_list) > ix &&
	id_list[ix] == 0 &&
	ob) {
	ret = ob->parse_command_id_list();
	if (arrayp(ret)) {
	    id_list[ix] = ret;
	    if (make_plural)
		pluid_list[ix] = map(ret, (: stringp($1) ? pluralize($1) : 0 :));
	} else {
	    id_list[ix] = 1;
	}
    }
    if (adjid_list &&
	sizeof(adjid_list) > ix &&
	adjid_list[ix] == 0 &&
	ob) {
	ret = ob->parse_command_adjectiv_id_list();
	if (arrayp(ret))
	    adjid_list[ix] =  ret;
	else
	    adjid_list[ix] = 1;
    }
}

mixed *parse_command(string cmd, mixed obarr, string pattern) {
    mixed *saved_ret = ret, *cret;

    ret = ({ });
    
    /* pattern and command cannot be empty */
    if (cmd == "" || pattern = "") return ({ });

    if (!stringp(cmd)) error("Bad argument 1 to parse_command().\n");
    if (!stringp(pattern)) error("Bad argument 3 to parse_command().\n");

    /* array of words in command */
    parse_warr = explode(cmd, " ");

    /* array of pattern elements */
    parse_patarr = explode(pattern, " ");
    
#ifndef __NO_ENVIRONMENT__
    if (objectp(obarr))
	obarr = ({ obarr }) + deep_inventory(obarr);
#endif
    if (!arrayp(obarr)) error("Bad argument 2 to parse_command().\n");
    
    id_list = allocate(sizeof(obarr));
    pluid_list = allocate(sizeof(obarr));
    adjid_list = allocate(sizeof(obarr));

    id_list_d = master()->parse_command_id_list();
    pluid_list_d = master()->parse_command_plural_id_list();
    adjid_list_d = master()->parse_command_adjectiv_id_list();
    prepos_list = master()->parse_command_prepos_list();
    
    allword = master()->parse_command_all_word();
    
    /*
     * Loop through the pattern. Handle %s but not '/'
     */
    for (six = 0, cix = 0, pix = 0; pix < sizeof(parse_patarr); pix++) {
	value = 0;
	fail = 0;
	
	if (parse_patarr[pix] == "%s") {
	    /*
	     * We are at end of pattern, scrap up the remaining words and put
	     * them in the fill-in value.
	     */
	    if (pix == sizeof(parse_patarr - 1)) {
		store_words_slice(six++, parse_warr, cix, sizeof(parse_warr) - 1);
		cix = sizeof(parse_warr);
	    } else {
		/*
		 * There is something after %s, try to parse with the next
		 * pattern. Begin with the current word and step one word for
		 * each fail, until match or end of words.
		 */
		ocix = fword = cix; /* current word */
		fpix = ++pix; /* pix == next pattern */
		do {
		    /*
		     * Parse the following pattern, fill-in values:
		     * stack_args[six] = result of %s stack_args[six + 1] =
		     * result of following pattern, if it is a fill-in
		     * pattern
		     */
		    fail = sub_parse(obarr, parse_patarr, ref pix,
				      parse_warr, ref cix);
		    if (fail) {
			cix = ++ocix;
			pix = fpix;
		    }
		} while (fail && (cix < sizeof(parse_warr)));
		
		/*
		 * We found something mathing the pattern after %s. First
		 * stack_args[six + 1] = result of match Then stack_args[six]
		 * = the skipped words before match
		 */
		if (!fail) {
		    if (value) { /* A match with a value fill in param */
			store_value(six + 1, value);
			store_words_slice(six, parse_warr, fword, ocix - 1);
			six += 2;
		    } else {	/* A match with a non value ie 'word' */
			store_words_slice(six++, parse_warr, fword, ocix - 1);
		    }
		    value = 0;
		}
	    }
	}
	/*
	 * The pattern was not %s, parse the pattern if it is not '/', a '/'
	 * here is skipped. If match, put in fill-in value.
	 */
	else if (parse_patarr[pix] != "/") {
	    fail = sub_parse(obarr, parse_patarr, ref pix, 
			      parse_warr, ref cix, ref fail);
	    if (!fail && value)
		store_value(six++, value);
	}
	/*
	 * Terminate parsing if no match
	 */
	if (fail)
	    break;
    }

    /*
     * Also fail when there is words left to parse and pattern exhausted
     */
    if (fail || cix < sizeof(parse_warr))
	return 0;

    cret = ret;
    ret = saved_ret;
    return cret;
}

private void
store_value(int pos, mixed what) {
    if (sizeof(ret) <= pos)
	ret += allocate(ret + 1 - pos);

    ret[pos] = what;
}

static void
store_words_slice(int pos, mixed *warr, int from, int to) {
    mixed *slice = warr[from..to];

    store_value(pos, implode(slice, " "));
}

private static int
sub_parse(mixed *obarr, mixed *patarr, int ref pix_in, 
	  mixed *warr, int ref cix_in)
{
    int cix, pix;
    int fail;
    
    /*
     * Fail if we have a pattern left but no words to parse
     */
    if (*cix_in == sizeof(warr))
	return 1;

    cix = *cix_in;
    pix = *pix_in;

    fail = one_parse(obarr, patarr[pix], warr, ref cix);

    while (fail) {
	pix++;
	cix = *cix_in;

	/*
	 * Find the next alternative pattern, consecutive '/' are skipped
	 */
	while (pix < sizeof(patarr) && patarr[pix] == "/") {
	    pix++;
	    fail = 0;
	}
	
	if (!fail && pix < sizeof(patarr)) {
	    fail = one_parse(obarr, patarr[pix], warr, ref cix);
	} else {
	    *pix_in = pix - 1;
	    return 1;
	}
    }

    /*
     * If there is alternatives left after the mathing pattern, skip them
     */
    if (pix + 1 < sizeof(patarr) && patarr[pix+1] == "/") {
	while (pix + 1 < sizeof(patarr) && patarr[pix+1] == "/") {
	    pix += 2;
	}
	if (pix >= sizeof(patarr))
	    pix = sizeof(patarr->size);
    }
    *cix_in = cix;
    *pix_in = pix;

    return fail;
}

private int
one_parse(mixed *obarr, string pat, mixed *warr, int ref cix_in)
{
    int ch, fail;
    string str1, str2;

    /*
     * Fail if we have a pattern left but no words to parse
     */
    if (*cix_in >= sizeof(warr))
	return 1;

    ch = pat[0];
    if (ch == '%')
	ch = pat[1];

    switch (ch) {
    case 'i':
    case 'I':
	fail = item_parse(obarr, warr, cix_in);
	break;

#ifndef __NO_ADD_ACTION__
    case 'l':
    case 'L':
	fail = living_parse(obarr, warr, cix_in);
	break;
#endif

    case 's':
    case 'S':
	value = 0;
	fail = 0;
	break;

    case 'w':
    case 'W':
	value = warr[*cix_in];
	(*cix_in)++;
	fail = 0;
	break;

    case 'o':
    case 'O':
	fail = single_parse(obarr, warr, cix_in);
	break;

    case 'p':
    case 'P':
	fail = prepos_parse(warr, cix_in);
	break;

    case 'd':
    case 'D':
	fail = number_parse(obarr, warr, cix_in);
	break;

    case '\'':
	str1 = pat[1..<2];
	str2 = warr[*cix_in];
	if (pat[<1] == '\'' && str1 == str2) {
	    fail = 0;
	    (*cix_in)++;
	} else
	    fail = 1;
	break;

    case '[':
	str1 = pat[1..<2];
	str2 = warr[*cix_in];
	if (str1 == str2)
	    (*cix_in)++;
	fail = 0;
	break;

    default:
	fail = 0;		/* Skip invalid patterns */
    }

    return fail;
}

string *ord1 = ({"", "first", "second", "third", "fourth", "fifth",
 "sixth", "seventh", "eighth", "nineth", "tenth",
 "eleventh", "twelfth", "thirteenth", "fourteenth",
 "fifteenth", "sixteenth", "seventeenth",
 "eighteenth", "nineteenth" });

string *ord10 = ({"", "", "twenty", "thirty", "forty", "fifty", "sixty",
 "seventy", "eighty", "ninety"});

string *sord10 = ({"", "", "twentieth", "thirtieth", "fortieth",
 "fiftieth", "sixtieth", "seventieth", "eightieth",
 "ninetieth"});

string *num1 = ({"", "one", "two", "three", "four", "five", "six",
 "seven", "eight", "nine", "ten",
 "eleven", "twelve", "thirteen", "fourteen", "fifteen",
 "sixteen", "seventeen", "eighteen", "nineteen"});

string *num10 = ({"", "", "twenty", "thirty", "forty", "fifty", "sixty",
 "seventy", "eighty", "ninety"});

private int
number_parse(mixed *obarr, mixed *warr, int ref cix_in) {
    int cix, ten, ones, num;
    string buf;
    
    cix = *cix_in;

    if (sscanf(warr[cix], "%d", num)) {
	if (num >= 0) {
	    (*cix_in)++;
	    value = num;
	    return 0;
	}
	return 1; /* only nonnegative numbers */
    }
    if (warr[cix] == allword) {
	(*cix_in)++;
	value = 0;
	return 0;
    }
    /* This next double loop is incredibly stupid. -Beek */
    for (ten = 0; ten < 10; ten++)
	for (ones = 0; ones < 10; ones++) {
	    buf = num10[ten] + (ten > 1 ? num1[ones] : num1[ten * 10 + ones]);
	    if (buf == warr[cix]) {
		(*cix_in)++;
		value = ten * 10 + ones;
		return 0;
	    }
	}

    /* this one too */
    for (ten = 0; ten < 10; ten++)
	for (ones = 0; ones < 10; ones++) {
	    buf = (ones ? ord10[ten] : sord10[ten]) + (ten > 1 ? ord1[ones] : ord1[ten*10 + ones]);
	    if (buf == warr[cix]) {
		(*cix_in)++;
		value = -(ten * 10 + ones);
		return 0;
	    }
	}

    return 1;
}

private int
item_parse(mixed *obarr, mixed *warr, int ref cix_in) {
    mixed *tmp, *ret;
    int cix, tix, obix, plur_flag, max_cix, match_all;

    tmp = allocate(sizeof(obarr) + 1);
    
    if (!number_parse(obarr, warr, cix_in)) {
	tmp[0] = value;
	match_all = (value == 0);
	plur_flag = (match_all || value > 1);
	have_number = 1;
	value = 0;
    } else {
	plur_flag = 0;
	match_all = 0;
    }

    for (max_cix = *cix_in, tix = 1, obix = 0; obix < sizeof(obarr); obix++) {
	cix = *cix_in;
	if (!objectp(obarr[obix]))
	    continue;
	if (cix == sizeof(warr) && match_all) {
	    tmp[tix++] = obarr[obix];
	    continue;
	}
	load_lpc_info(obix, obarr[obix]);

	if (match_object(obix, warr, ref cix, ref plur_flag)) {
	    tmp[tix++] = obarr[obix];
	    max_cix = (max_cix < cix) ? cix : max_cix;
	}
    }

    if (tix < 2) {
	if (have_number)
	    (*cix_in)--;
	return 1;
    } else {
	if (*cix_in < sizeof(warr))
	    *cix_in = max_cix + 1;
	if (!have_number)
	    tmp[0] = !plur_flag;
	
	value = tmp[0..tix-1];
	return 0;
    }
}

#ifndef __NO_ADD_ACTION__
private int
living_parse(mixed *obarr, array warr, int ref cix_in, int ref fail)
{
    mixed *live;
    object ob;
    int obix, tix;

    live = allocate(sizeof(obarr));
    tix = 0;

    for (obix = 0; obix < sizeof(obarr); obix++)
	if (living(obarr[obix]))
	    live[tix++] = obarr[obix];

    if (tix && !item_parse(live, warr, cix_in))
	return 0;

    ob = find_player(warr[*cix_in]);
    if (!ob)
	ob = find_living(warr[*cix_in]);

    if (ob) {
	value = ob;
	(*cix_in)++;
	return 0;
    }
    return 1;
}
#endif

private int
single_parse(mixed *obarr, mixed *warr, int ref cix_in)
{
    int cix, obix, plur_flag;

    for (obix = 0; obix < sizeof(obarr); obix++) {
	cix = *cix_in;
	if (objectp(obarr[obix]))
	    load_lpc_info(obix, obarr[obix]);
	plur_flag = 0;
	if (match_object(obix, warr, ref cix, ref plur_flag)) {
	    *cix_in = cix + 1;
	    value = obarr[obix];
	    return 0;
	}
    }
    return 1;
}

private int
prepos_parse(mixed *warr, int ref cix_in, mixed prepos) {
    mixed *tarr;
    string tmp;
    int pix, tix;

    if (!prepos || !arrayp(prepos))
	prepos = prepos_list;

    for (pix = 0; pix < sizeof(prepos); pix++) {
	if (!stringp(prepos[pix]))
	    continue;

	tmp = prepos[pix];
	if (member_array(' ', tmp) == -1) {
	    if (tmp == warr[*cix_in]) {
		(*cix_in)++;
		break;
	    }
	} else {
	    tarr = explode(tmp, " ");
	    if (*cix_in + sizeof(tarr) <= sizeof(warr)) {
		for (tix = 0; tix < sizeof(tarr); tix++) {
		    if (*cix_in + tix >= sizeof(warr) ||
			warr[*cix_in + tix] != tarr[tix])
			break;
		}
		if (tix == sizeof(tarr)) {
		    (*cix_in) += sizeof(tarr);
		    break;
		}
	    }
	}
    }

    if (pix == sizeof(prepos)) {
	value = 0;
	return 1;
    } else {
	value = prepos[pix];
	return 0;
    }
}

private int
match_object(int obix, mixed *warr, int ref cix_in, int ref plur) {
    mixed *ids;
    int il, pos, cplur, old_cix;
    string str;

    for (cplur = (*plur * 2); cplur < 4; cplur++) {
	switch (cplur) {
	case 0:
	    if (!id_list_d)
		continue;
	    ids = id_list_d;
	    break;

	case 1:
	    if (!d_list ||
		sizeof(id_list) <= obix ||
		!arrayp(id_list[obix]))
		continue;
	    ids = id_list[obix];
	    break;

	case 2:
	    if (!pluid_list_d)
		continue;
	    ids = pluid_list_d;
	    break;

	case 3:
	    if (!pluid_list ||
		sizeof(gpluid_list) <= obix ||
		!arrayp(gpluid_list[obix]))
		continue;
	    ids = pluid_list[obix];
	    break;

	default:
	    ids = 0;
	}

	for (il = 0; il < sizeof(ids); il++) {
	    if (stringp(ids[il])) {
		str = ids[il];	/* A given id of the object */
		old_cix = *cix_in;
		if ((pos = find_string(str, warr, cix_in)) >= 0) {
		    if (pos == old_cix) {
			if (cplur > 1)
			    *plur = 1;
			return 1;
		    } else if (check_adjectiv(obix, warr, old_cix, pos - 1)) {
			if (cplur > 1)
			    *plur = 1;
			return 1;
		    }
		}
		*cix_in = old_cix;
	    }
	}
    }
    return 0;
}

static int
find_string(string str, mixed *warr, int ref cix_in)
{
    int fpos;
    string p1;
    mixed *split;

    for (; *cix_in < warr->size; (*cix_in)++) {
	p1 = warr[*cix_in];

	if (p1 == str)	/* str was one word and we found it */
	    return *cix_in;

	if (member_array(' ', str) == -1)
	    continue;

	/*
	 * If str was multi word we need to make some special checks
	 */
	if (*cix_in == sizeof(warr->size) - 1)
	    continue;

	split = explode(str, " ");

	/*
	 * warr->size - *cix_in ==	
	 * 2: One extra word 
	 * 3: Two extra words
	 */
	if (sizeof(split) > sizeof(warr) - *cix_in)
	    continue;

	fpos = *cix_in;
	for (; (*cix_in - fpos) < sizeof(split); (*cix_in)++) {
	    if (split[*cix_in - fpos] == warr[*cix_in])
		break;
	}
	if ((*cix_in - fpos) == sizeof(split))
	    return fpos;

	*cix_in = fpos;
    }
    return -1;
}

private int
check_adjectiv(int obix, mixed *warr, int from, int to)
{
    int il, back, fail;
    string adstr;
    mixed *ids;

    if (arrayp(adjid_list[obix]))
	ids = adjid_list[obix];
    else
	ids = 0;

    for (fail = 0, il = from; il <= to; il++) {
	if ((member_array(warr[il], ids) < 0) &&
	    (member_array(warr[il], adjid_list_d) < 0))
	    fail = 1;
    }

    /*
     * Simple case: all adjs were single word
     */
    if (!fail)
	return 1;

    if (from == to)
	return 0;

    /*
     * If we now have: "adj1 adj2 adj3 ... adjN"
     * We must test in order: "adj1 adj2 adj3 .... adjN-1 adjN"
                              "adj1 adj2 adj3 .... adjN-1"
			      "adj1 adj2 adj3 ...." 
			      ....
     * if match for adj1 .. adj3 continue with:
     *                        "adj4 adj5 .... adjN-1 adjN"
     *                        "adj4 adj5 .... adjN-1"
     *                        "adj4 adj5 ...."
     *                        .....
     */
    for (il = from; il <= to;) { /* adj1 .. adjN */
	for (back = to; back >= il; back--) {	/* back from adjN to adj[il] */
	    /*
	     * Create teststring with "adj[il] .. adj[back]"
	     */
	    adstr = "";
	    for (sum = il; sum <= back; sum++) {	/* test "adj[il] ..
							 * adj[back]" */
		if (sum > il)
		    adstr += " ";
		adstr += warr[sum];
	    }
	    if ((member_array(adstr, ids) < 0) &&
		(member_array(adstr, adjid_list_d) < 0))
		continue;
	    else {
		il = back + 1;	/* Match "adj[il] adj[il+1] .. adj[back]" */
		back = to;
		break;
	    }
	}
	if (back < to)
	    return 0;
    }
    return 1;
}
