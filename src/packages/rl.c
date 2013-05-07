#include "../lpc_incl.h"
#include "../efun_protos.h"
#include "../port.h"
#include "dumphb.c"

void get_objects_hb (object_t *** list, int * size, get_objectsfn_t callback, void * data);

#ifdef F_ROLL
void
f_roll() 
{
    long roll = 0;

    if ( (sp - 1)->u.number > 0 && sp->u.number > 0 ) {
      while( (sp - 1)->u.number-- )
      roll += 1 + random_number( sp->u.number );
    }

    pop_stack(); // Pop one...
    sp->u.number = roll; // And change the other!
}
#endif

#ifdef F_DUMPALLHB
void
f_dumpallhb(void)
{
	if(st_num_arg) {
		dumphb(sp->u.string);
		free_string_svalue(sp--);
	} 
	else
		dumphb("/OBJ_DUMPHB");
}
#endif

#ifdef F_OBJECTS_HB
void
f_objects_hb(void)
{
	array_t *heart_beats;
	int count, i;
	const char *func = 0;
    object_t **list;
    array_t *ret;
    funptr_t *f = 0;
	int num_arg = st_num_arg;

   if (!num_arg) func = 0;
   else if (sp->type == T_FUNCTION) f = sp->u.fp;
   else func = sp->u.string;

	get_objects_hb(&list, &count, 0, 0);

	if (f || func) {
    /* NOTE: If an object's hidden status changes during a callback, that
     * change will NOT be reflected in the returned array.  If the caller
     * is destructed in a callback or the callback function does not exist,
     * abort and return the_null_array.
     */
    for (i = 0;  i < count;  i++) {
      svalue_t *v;

      push_object(list[i]);
      if (f) {
        v = call_function_pointer(f, 1);
      } else {
        v = apply(func, current_object, 1, ORIGIN_EFUN);
      }
      if (!v || (current_object->flags & O_DESTRUCTED)) {
        pop_n_elems(num_arg + 1);
        push_refed_array(&the_null_array);
        return;
      }
      if (v->type == T_NUMBER && !v->u.number) {
        list[i] = 0;
      }
    }
    for (i = 0;  i < count;  i++) {
      if (!list[i] || (list[i]->flags & O_DESTRUCTED)) {
        list[i--] = list[--count];
      }
    }
  }
  
	if (count > max_array_size)
        count = max_array_size;
    ret = allocate_empty_array(count);
    for (i = 0;  i < count;  i++) {
        ret->item[i].type = T_OBJECT;
        ret->item[i].u.ob = list[i];
        add_ref(list[i], "f_objects");
   }

   pop_n_elems(num_arg + 1); /* include our temporary 'string' */
   push_refed_array(ret);
}
#endif

void get_objects_hb (object_t *** list, int * size, get_objectsfn_t callback, void * data)
{
	array_t *heart_beats;
	object_t *ob;

	// Obtenemos los objetos con HB.
	heart_beats = get_heart_beats();
	*list = (object_t **)new_string((heart_beats->size * sizeof(object_t *)) - 1, "get_objects_hb");

	if (!*list)
		fatal("Out of memory!\n");

	push_malloced_string((char *)*list);

    for (*size = 0;  *size < heart_beats->size; (*size)++) {
    	ob = heart_beats->item[*size].u.ob;

        if (!callback || callback(ob, data))
            (*list)[*size] = ob;
    }
}