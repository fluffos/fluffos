#include <sys/types.h>
#include <sys/time.h>

#include "lint.h"
#include "config.h"

#ifdef sun
time_t time PROT((time_t *));
#endif


/*
 * This file defines things that may have to be changem when porting
 * LPmud to new environments. Hopefully, there are #ifdef's that will take
 * care of everything.
 */

/*
 * Return a random number in the range 0 .. n-1
 */
int random_number(n)
    int n;
{
#ifdef RANDOM
    return random() % n;
#else /* RANDOM */
#ifdef DRAND48
    return (int)(drand48() * n);
#else /* DRAND48 */
    extern int current_time;

    return current_time % n;	/* Suit yourself :-) */
#endif /* DRAND48 */
#endif /* RANDOM */
}

/*
 * The function time() can't really be trusted to return an integer.
 * But this game uses the 'current_time', which is an integer number
 * of seconds. To make this more portable, the following functions
 * should be defined in such a way as to retrun the number of seconds since
 * some chosen year. The old behaviour of time(), is to return the number
 * of seconds since 1970.
 */

int get_current_time() {
    return (int)time(0l);	/* Just use the old time() for now */
}

char *time_string(t)
    int t;
{
    return (char *)ctime((time_t *)&t);
}
