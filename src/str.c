#include <stdio.h>
#include <ctype.h>

#define DIGIT(x)	(isdigit(x) ? (x) - '0' : \
			islower(x) ? (x) + 10 - 'a' : (x) + 10 - 'A')
#define MBASE	('z' - 'a' + 1 + 10)

long
strtol(str, ptr, base)
register char *str;
char **ptr;
register int base;
{
	register long val;
	register int c;
	int xx, neg = 0;

	if (ptr != (char **)0)
		*ptr = str; /* in case no number is formed */
	if (base < 0 || base > MBASE)
		return (0); /* base is invalid -- should be a fatal error */
	if (!isalnum(c = *str)) {
		while (isspace(c))
			c = *++str;
		switch (c) {
		case '-':
			neg++;
		case '+': /* fall-through */
			c = *++str;
		}
	}
	if (base == 0)
		if (c != '0')
			base = 10;
		else if (str[1] == 'x' || str[1] == 'X')
			base = 16;
		else
			base = 8;
	/*
	 * for any base > 10, the digits incrementally following
	 *	9 are assumed to be "abc...z" or "ABC...Z"
	 */
	if (!isalnum(c) || (xx = DIGIT(c)) >= base)
		return (0); /* no number formed */
	if (base == 16 && c == '0' && isxdigit(str[2]) &&
	    (str[1] == 'x' || str[1] == 'X'))
		c = *(str += 2); /* skip over leading "0x" or "0X" */
	for (val = -DIGIT(c); isalnum(c = *++str) && (xx = DIGIT(c)) < base; )
		/* accumulate neg avoids surprises near MAXLONG */
		val = base * val - xx;
	if (ptr != (char **)0)
		*ptr = str;
	return (neg ? val : -val);
}

strcspn(s, set)
register char *s;
char *set;

{
register char *t;
register int count = 0;
	
	while (*s)
	{
		t = set;
		while (*t && (*s != *t)) t++;
		if (!*t)
		{
			s++;
			count++;
		}
		else
			break;
	}
	
	return (count);
}

    
char *memset (s, c, n)
     char *s;
     int c, n;
{
   if(c == 0)
      bzero(s, n);
   else{
      fprintf(stderr, "Failed in memset\n");
      exit(1);
   }
}

char *memcpy(b,a,s)
    char *b, *a;
    int s;
{
    bcopy(a,b,s);
    return b;
}
