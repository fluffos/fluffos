#include "../lpc_incl.h"

/*
	Function that removes blank spaces left and / or right,
*/
char *trim(const char *str, int left, int right){
	const char *strI, *strF;
	char *ret;
	int size;
		
	// Get Start & Final Positions.
	strI = str;
	strF = strI + strlen(strI) - 1;

	// Skip starts blank spaces in left mode.
	while(left && isspace (*strI))
		strI++;

	// No text.
	if(strI >= strF){
		ret = new_string(1, "trim:empty");
		ret[0] = '\0';
		return ret;
	}

	// Skip finals blank spaces in right mode.
	while(right && isspace (*strF))
		strF--;

	// We prepare a substring without blank spaces and return it.
	size = strF - strI + 1;
	ret = new_string(size, "f_trim");
	strncpy(ret, strI, size);
	ret[size] = '\0';
	return ret;
}

#ifdef F_TRIM
void
f_trim(void)
{
	const char *str;

	// If parameter isn't string, we return "".
	if(sp->type != T_STRING){
		put_constant_string("");
		return;
	}
	
	// Parameter is string.
		// Unlink for modify string withoud modify origin string.
	unlink_string_svalue(sp);

	// Remove blank spaces.
	str = trim(sp->u.string, 1, 1);

	// Update string.
	sp->u.string = str;
}
#endif

#ifdef F_LTRIM
void
f_ltrim(void)
{
	const char *str;

	// If parameter isn't string, we return "".
	if(sp->type != T_STRING){
		put_constant_string("");
		return;
	}
	
	// Parameter is string.
		// Unlink for modify string withoud modify origin string.
	unlink_string_svalue(sp);

	// Remove blank spaces.
	str = trim(sp->u.string, 1, 0);

	// Update string.
	sp->u.string = str;
}
#endif

#ifdef F_RTRIM
void
f_rtrim(void)
{
	const char *str;

	// If parameter isn't string, we return "".
	if(sp->type != T_STRING){
		put_constant_string("");
		return;
	}
	
	// Parameter is string.
		// Unlink for modify string withoud modify origin string.
	unlink_string_svalue(sp);

	// Remove blank spaces.
	str = trim(sp->u.string, 0, 1);

	// Update string.
	sp->u.string = str;
}
#endif
