string parser_error_message(int kind, object ob, mixed arg, int flag) {
    string ret;
    if (ob) 
	ret = ob->short() + ": ";
    else
	ret = "";
    
    switch (kind) {
    case 1:
	if (flag)
	    return ret + "There is no such " + arg + " here.\n";
	else
	    return ret + "There is no " + arg + " here.\n";
	break;
    case 2:
	if (flag)
	    return ret + "None of the " + pluralize(arg) + " are alive.\n";
	else
	    return ret + "The " + arg + " isn't alive.\n";
	break;
    case 3:
	if (flag)
	    return ret + "You can't reach them.\n";
	else
	    return ret + "You can't reach it.\n";
	break;
    case 4:
	{
	    array descs = unique_array(arg, (: $1->the_short() :));
	    string str;
	    
	    if (sizeof(descs) == 1)
		return ret + "Which " + descs[0][0]->short() + " do you mean?\n";
	    str = ret + "Do you mean ";
	    for (int i = 0; i < sizeof(descs); i++) {
		if (sizeof(descs[i]) > 1)
		    str += "one of ";
		str += descs[i][0]->the_short();
		if (i == sizeof(descs) - 1)
		    str += " or ";
		else 
		    str += ", ";
	    }
	    return str + "?\n";
	}
	break;
    case 5:
	if (arg > 1)
	    return ret + "There are only " + arg + " of them.\n";
	else
	    return ret + "There is only one of them.\n";
	break;
    case 6:
	return ret + arg;
    case 7:
	return ret + "There is no " + arg + " here.\n";
    case 8:
	return ret + "You can't use more than one object at a time with that verb.\n";
    }
}
