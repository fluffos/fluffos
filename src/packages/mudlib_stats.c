#ifdef LATTICE
#include "/lpc_incl.h"
#include "/mudlib_stats.h"
#else
#include "../lpc_incl.h"
#include "../mudlib_stats.h"
#endif

#ifndef NO_MUDLIB_STATS
#ifdef F_DOMAIN_STATS
void
f_domain_stats PROT((void))
{
    struct mapping *m;

    if (st_num_arg) {
        m = get_domain_stats(sp->u.string);
        free_string_svalue(sp--);
    } else {
        m = get_domain_stats(0);
    }
    if (!m) {
        push_number(0);
    } else {
        /* ref count is properly decremented by get_domain_stats */
        push_mapping(m);
    }
}
#endif

#ifdef F_SET_AUTHOR
void
f_set_author PROT((void))
{
    set_author(sp->u.string);
}
#endif

#ifdef F_AUTHOR_STATS
void
f_author_stats PROT((void))
{
    struct mapping *m;

    if (st_num_arg) {
        m = get_author_stats(sp->u.string);
        free_string_svalue(sp--);
    } else {
        m = get_author_stats(0);
    }
    if (!m) {
        push_number(0);
    } else {
        /* ref count is properly decremented by get_author_stats */
        push_mapping(m);
    }
}
#endif
#endif
