#ifndef ADD_ACTION_H
#define ADD_ACTION_H 1

#ifndef NO_ADD_ACTION
extern object_t *hashed_living[CFG_LIVING_HASH_SIZE];

int	    parse_command	PROT((char *, object_t *));
void	    clear_notify	PROT((object_t *));
void	    stat_living_objects	PROT((outbuffer_t *));
void	    remove_living_name	PROT((object_t *));
object_t *  find_living_object	PROT((char *, int));
void	    setup_new_commands	PROT((object_t *, object_t *));
#ifndef NO_ENVIRONMENT
void	    remove_sent		PROT((object_t *, object_t *));
#endif
#else
/* STUBS */
#define parse_command(x, y)	    do{}while(0)
#define clear_notify(x)		    do{}while(0)
#define stat_living_objects(x)	    do{}while(0)
#define remove_living_name(x)	    do{}while(0)
#define remove_sent(x, y)	    do{}while(0)
#define setup_new_commands(x, y)    do{}while(0)
#endif

#endif	/* ADD_ACTION_H */
