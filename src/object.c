#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#ifdef __STDC__
#include <memory.h>
#endif

#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "mapping.h"
#include "object.h"
#include "sent.h"
#include "exec.h"
#include "debug.h"

extern int d_flag;
extern int total_num_prog_blocks, total_prog_block_size;

extern char *xalloc PROT((int)), *string_copy PROT((char *));

void remove_swap_file PROT((struct object *));

extern int atoi();

struct object *previous_ob;
extern struct svalue const0, const0n;

int tot_alloc_object, tot_alloc_object_size;
int save_svalue_depth = 0;

static char *save_array PROT((struct vector *v));
char *save_mapping PROT ((struct mapping *m));
static int vector_save_size PROT((struct vector *));
static struct mapping *restore_mapping PROT((char **));

/*
 * Replace newlines in a string with a carriage return, to make the string
 * writeable on one line.
 */

static void replace_newline(str)
    char *str;
{
    for (; *str; str++) {
	if (str[0] == '\n')
#ifndef MSDOS
	    str[0] = '\r';
#else
	    str[0] = 30;
#endif
    }
}

/*
 * Replace carriage return in a string with newlines.
 */

static void restore_newline(str)
    char *str;
{
    for (; *str; str++) {
#ifndef MSDOS
	if (str[0] == '\r')
#else
	if (str[0] == 30)
#endif
	    str[0] = '\n';
    }
}

static int my_strlen(str)
    char *str;
{
    int sz = 0;

    while (*str) {
	sz++; 
	if (*str == '\"' || *str == '\\') sz++;
	str++;
    }
    return sz;
}

/*
 * Similar to strcat(), but escapes all funny characters.
 * Used by save_object().
 * src is modified temporarily, but restored again :-(
 */
static void my_strcat(dest,src)
    char *dest,*src;
{
    char *pt,*pt2,ch[2];

    pt = strchr(src,'\\'); ch[1] = 0;
    pt2 = strchr(src,'\"');
    if ((pt2 && pt2 < pt) || (pt == 0))
	pt = pt2;
    while (pt) {
	ch[0] = *pt; *pt = 0; 
	strcat(dest,src); strcat(dest,"\\"); strcat(dest,ch);
	src = pt+1; *pt = ch[0];
	pt = strchr(src,'\\'); 
	pt2 = strchr(src,'\"');
	if ((pt2 && pt2 < pt) || (pt == 0))
	    pt = pt2;
    }
    strcat(dest,src);
}
	
int svalue_save_size(v)
   struct svalue *v;
{
	char numbuf[SMALL_STRING_SIZE];
	int mapping_save_size PROT((struct mapping *));
	int size;

	if (v->type == T_STRING)
		return my_strlen(v->u.string) + 3;  /* my_ */
	if (v->type == T_POINTER) {
		save_svalue_depth++;
		if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
			return 0;
		}
		size = vector_save_size(v->u.vec);
		save_svalue_depth--;
		return 2 + size + 2 + 1;
	}
	if (v->type == T_MAPPING) {
		save_svalue_depth++;
		if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
			return 0;
		}
		size = mapping_save_size(v->u.map);
		save_svalue_depth--;
		return size + 5;
	}
	if (v->type == T_NUMBER) {
		sprintf(numbuf, "%d", v->u.number);
		return strlen(numbuf) + 1;
	}
	return 2;
}

static int vector_save_size(v)
struct vector *v;
{
	int i, siz;

	for (i=0, siz = 0; i < v->size; i++)
		siz += svalue_save_size(&v->item[i]);
	return siz;
}

void save_svalue(v, buf)
struct svalue *v;
char *buf;
{
    char *tbuf;

    if (v->type == T_STRING) {
       strcat(buf, "\""); my_strcat(buf, v->u.string);   /* my_ */
       strcat(buf, "\""); 
    } else if (v->type == T_POINTER) {
       tbuf = save_array(v->u.vec);
       strcat(buf, tbuf);  FREE(tbuf);
    } else if (v->type == T_NUMBER) {
       sprintf(buf + strlen(buf), "%d", v->u.number);
	} else if (v->type == T_MAPPING) {
       tbuf = save_mapping(v->u.map);
       strcat(buf,tbuf);   FREE(tbuf);
    }
}

/*
 * Encode an array of elements into a contiguous string.
 */
static char *
save_array(v)
     struct vector *v;
{
    char *buf;
    int i;
    
    buf = DXALLOC(2+vector_save_size(v)+2+1, 4096, "save_array");
    
    strcpy(buf,"({");
    for (i=0; i < v->size; i++) {
        save_svalue(&v->item[i], buf);
        strcat(buf, ",");
    }
    strcat(buf,"})");
    return buf;
}

/*
 * Save an object to a file.
 * The routine checks with the function "valid_write()" in /obj/master.c
 * to assertain that the write is legal.
 */
void save_object(ob, file)
    struct object *ob;
    char *file;
{
    char *name, tmp_name[80];
    int len, i;
    FILE *f;
    int failed = 0;
    /* struct svalue *v; */

    if (ob->flags & O_DESTRUCTED)
	return;
    file = check_valid_path(file, ob, "save_object", 1);
    if (file == 0)
	error("Denied write permission in save_object().\n");
    len = strlen(file);
    name = DXALLOC(len + 3, 4096, "save_object: 1");
    (void)strcpy(name, file);
#ifndef MSDOS
    (void)strcat(name, ".o");
#endif
    /*
     * Write the save-files to different directories, just in case
     * they are on different file systems.
     */
    sprintf(tmp_name, "%s.tmp", name);
#ifdef MSDOS
    (void)strcat(name, ".o");
#endif
    f = fopen(tmp_name, "w");
    if (f == 0) {
	FREE(name);
	error("Could not open %s for a save.\n", tmp_name);
    }
	for (i=0; (unsigned)i < ob->prog->p.i.num_variables; i++) {
		struct svalue *v = &ob->variables[i];
		char *new_string;
		int theSize;

		if (ob->prog->p.i.variable_names[i].type & TYPE_MOD_STATIC)
			continue;

		save_svalue_depth = 0;
		theSize = svalue_save_size(v);
		if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
		error("Mappings and/or arrays nested too deep (%d) for save_object\n",
			MAX_SAVE_SVALUE_DEPTH);
		}
		new_string = (char *)DXALLOC(theSize, 4096, "save_object: 2");
		*new_string = '\0';
		save_svalue(v, new_string);
		replace_newline(new_string);
		if (fprintf(f, "%s %s\n", ob->prog->p.i.variable_names[i].name,
				new_string) == EOF) {
			failed = 1;
		}
		FREE(new_string);
	}
    (void)unlink(name);
#ifndef MSDOS
    if (link(tmp_name, name) == -1)
#else
    (void) fclose(f);
    if (rename(tmp_name,name) < 0)
#endif
    {
	perror(name);
	printf("Failed to link %s to %s\n", tmp_name, name);
	add_message("Failed to save object !\n");
    }
#ifndef MSDOS
    (void)fclose(f);
    unlink(tmp_name);
#endif
    FREE(name);
    if (failed)
	add_message("Failed to save to file. Disk could be full.\n");
}

static char *
my_string_copy(str)
char *str;
{
	char *apa, *cp;

	cp = apa = DXALLOC(strlen(str)+1, 4096, "my_string_copy");
    
	while (*str) {
		if (*str == '\\') {
			*cp = str[1];
			if (str[1]) {
				str+=2;
			} else {
				str++;   /* String ends with a \\ buggy probably */
			}
			cp++;
		} else {
			*cp = *str; cp++; str++;
		}
	}
	*cp = 0;
	cp = string_copy(apa);
	FREE(apa);
	return cp;
}

/*
 * Find the size of an array or mapping. Return -1 for failure.
 */
static int restore_size(str, is_mapping)
char **str;
int is_mapping;
{
  char *pt, *pt2;
  int siz,tsiz;

  pt = *str; 
  if (strncmp(pt, is_mapping ? "([" : "({", 2)) return -1;
  else pt += 2;
  siz = 0;

  while ((pt) && (*pt)) {
    if (pt[0] == '}') {
      if (pt[1] != ')') return -1;
      if (is_mapping) return -1;
      *str = &pt[2];
      return siz;
    }
    if (pt[0] == ']') {
      if (pt[1] != ')') return -1;
      if (!is_mapping) return -1;
      *str = &pt[2];
      return siz;
    }
	if (pt[0] == '\"') {
		for (pt2 = &pt[1]; *pt2 && (*pt2 != '\"'); pt2++) {
			if (*pt2 == '\\') {
				pt2++;
			}
		}
		if (!*pt2) {
			return -1;
		}
		pt2--;
      if (pt2[2] != ',' && pt2[2] != ':') return -1;
      siz++;
      pt = &pt2[3];
    }
    else if (pt[0] == '(') { 
      tsiz = restore_size(&pt, pt[1] == '[');
            /* Lazy way of doing it, a bit inefficient */
      if (tsiz < 0)
	  return -1;
      pt++;
      siz++;
    }
    else {
      if (is_mapping && !(siz % 2))
         pt2 = strchr(pt, ':');
      else
         pt2 = strchr(pt, ',');
      if (!pt2)
	  return -1;
      siz++;
      pt = &pt2[1];
    }
  }
  return -1;
}

static struct vector *
restore_array(str, is_mapping)
     char **str;
     int is_mapping;
{
  struct vector *v,*t;
  char *pt,*pt2;
  char delim;
  int i,siz;
  
  pt = *str; 
  if (is_mapping && strncmp(pt, "([", 2)) return 0;
  else if (!is_mapping && strncmp(pt, "({", 2)) return 0;
  pt2 = pt;
  siz = restore_size(&pt2,is_mapping);
  if (siz < 0) return 0;
  v = allocate_array(siz);
  pt+=2;

  for (i=0;i<siz;i++) {
    if (!*pt) return v;
    if (is_mapping && !(i % 2)) delim = ':';
    else delim = ',';
    if (pt[0] == '\"') {
	for (pt2 = &pt[1]; *pt2 && (*pt2 != '\"'); pt2++) {
		if (*pt2 == '\\') {
			pt2++;
		}
	}
	if (!*pt2) {
		return  v;
	}
	pt2--;
      if (pt2[2] != delim) return v;
      pt2[1] = 0;
      v->item[i].type = T_STRING;
      v->item[i].u.string = my_string_copy(pt+1); /* my_ */
      v->item[i].subtype = STRING_MALLOC;
      pt = &pt2[3];
    }
    else if (pt[0] == '(' && pt[1] == '{') {
      t = restore_array(&pt, 0);
      if (!t) return v;
      v->item[i].type = T_POINTER;
      v->item[i].u.vec = t;
      /* v->item[i].u.vec->ref++; marion - ref is already 1 (allocate_array) */
      pt++;
    } else if (pt[0] == '(' && pt[1] == '[') {
      struct mapping *m;
      m = restore_mapping(&pt);
      if (!m) return v;
      v->item[i].type = T_MAPPING;
      v->item[i].u.map = m;
      pt++;
    }
    else {
      pt2 = strchr(pt, delim);
      if (!pt2) return v;
      pt2[0] = 0;
      v->item[i].type = T_NUMBER;
      sscanf(pt,"%d",&(v->item[i].u.number));
      pt = &pt2[1];
    }
  }
  if (is_mapping && strncmp(pt, "])", 2)) return v;
  if (!is_mapping && strncmp(pt, "})", 2)) return v;
  *str = &pt[2];
  return v;
}

static struct mapping
*restore_mapping(str)
char **str;
{
   struct vector *v;
   struct mapping *newmap;

   /* We take the easy way out. */
   v = restore_array(str, 1);
   if (!v) return (struct mapping *) 0;
   newmap = load_mapping_from_aggregate(&v->item[0], v->size);
   free_vector(v);
   return newmap;
}

int
restore_svalue(val, v)
char *val;
struct svalue *v;
{
	if (val[0] == '\"') {
	    restore_newline(val+1);
	    free_svalue(v);
	    v->type = T_STRING;
		val[strlen(val) - 1] = '\0'; /* don't copy the ending quote (") */
	    v->u.string = my_string_copy(val+1);
	    v->subtype = STRING_MALLOC;
	    return 0;
	}
/* Restore array: JnA 910520
*/
	if (val[0] == '(' && val[1] == '{') {
	    char *pt = val;
	    restore_newline(val+1);
	    free_svalue(v);
	    v->type = T_POINTER;
            v->u.vec = restore_array(&pt, 0);
	    if (!v->u.vec) {
			*v = const0n;
			return -1;
	    }
	    return 0;
	}
/* Restore mapping: JTR 01/24/92
*/ 
        if (val[0] == '(' && val[1] == '[') {
            char *pt = val;
            restore_newline(val + 1);
            free_svalue(v);
            v->type = T_MAPPING;
            v->u.map = restore_mapping(&pt);
            if (!v->u.map) {
               *v = const0n;
				return -2;
            }
            return 0;
        }

	free_svalue(v);
	v->type = T_NUMBER;
	v->u.number = atoi(val);
	return 0;
}

void
restore_object_from_buff(ob, theBuff, name, val)
struct object *ob;
char *theBuff, *name, *val;
{
	char *buff, *nextBuff, *tmp, *theEnd, *space;
	char var[100];
    struct variable *p;
	int rc;

	theEnd = theBuff + strlen(theBuff);
	nextBuff = theBuff;
    while (1) {
	struct svalue *v;

	buff = nextBuff;
	if (!buff || (buff == theEnd))
	    break;
	if ((tmp = strchr(buff, '\n'))) {
		*tmp = '\0';
		nextBuff = tmp + 1;
	} else {
		nextBuff = 0;
	}
	space = strchr(buff, ' ');
	if ((space == 0) || ((space - buff) >= sizeof(var))) {
	    error("Illegal format when restore %s.\n", name);
	}
	(void)strncpy(var, buff, space - buff);
	var[space - buff] = '\0';
	(void)strcpy(val, space+1);
	p = find_status(var, 0);
	if (p == 0 || (p->type & TYPE_MOD_STATIC))
	    continue;
	v = &ob->variables[p - ob->prog->p.i.variable_names];
    rc = restore_svalue(val, v);
	if (rc < 0) {
		FREE(val);	
		FREE(name);
		FREE(theBuff);
		if (rc == -1)
 	        error("Illegal array format when restore %s.\n", name);
		else if (rc == -2)
            error("Illegal mapping format when restore %s.\n", name);
	}
    }
}

int restore_object(ob, file)
    struct object *ob;
    char *file;
{
    char *name, *val, *theBuff;
    int len;
    FILE *f;
    struct object *save = current_object;
    struct stat st;

    if (current_object != ob)
	fatal("Bad argument to restore_object()\n");
    if (ob->flags & O_DESTRUCTED)
	return 0;

    file = check_valid_path(file, ob, "restore_object: 1", 0);
    if (file == 0)
	error("Denied read permission in restore_object().\n");

    len = strlen(file);
    name = DXALLOC(len + 3, 4096, "restore_object: 2");
    (void)strcpy(name, file);
    if (name[len-2] == '.' && name[len-1] == 'c')
	name[len-1] = 'o';
    else
	(void)strcat(name, ".o");
    f = fopen(name, "r");
    if (!f || fstat(fileno(f), &st) == -1) {
	FREE(name);
	if (f) 
	    (void)fclose(f);
	return 0;
    }
    if (st.st_size == 0) {
	(void)fclose(f);
	FREE(name);
	return 0;
    }
    val = DXALLOC(st.st_size + 1, 4096, "restore_object: 3");
    theBuff = DXALLOC(st.st_size + 1, 4096, "restore_object: 4");
    fread(theBuff, 1, st.st_size, f);
	fclose(f);
	theBuff[st.st_size] = '\0';
    current_object = ob;
 	restore_object_from_buff(ob, theBuff, name, val);
    current_object = save;
    if (d_flag > 1)
	debug_message("Object %s restored from %s.\n", ob->name, name);
    FREE(name);
    FREE(theBuff);
    FREE(val);
    return 1;
}

void tell_npc(ob, str)
    struct object *ob;
    char *str;
{
	push_constant_string(str);
	(void)apply("catch_tell", ob, 1);
}

/*
 * tell_object: Send a message to an object.
 * If it is an interactive object, it will go to his
 * screen. Otherwise, it will go to a local function
 * catch_tell() in that object. This enables communications
 * between users and NPC's, and between other NPC's.
 * If INTERACTIVE_CATCH_TELL is defined then the message always
 * goes to catch_tell unless the target of tell_object is interactive
 * and is the current_object in which case it is written via add_message().
 */
void tell_object(ob, str)
    struct object *ob;
    char *str;
{
	struct object *save_command_giver;

	save_command_giver = command_giver;
	if (!ob || (ob->flags & O_DESTRUCTED)) {
		command_giver = 0;
		add_message("%s", str);
		command_giver = save_command_giver;
		return;
	}
#ifdef INTERACTIVE_CATCH_TELL
	tell_npc(ob, str);
	return;
#else
	if (ob->interactive) {
		command_giver = ob;
		add_message("%s", str);
		command_giver = save_command_giver;
		return;
	}
	tell_npc(ob, str);
#endif
}

void free_object(ob, from)
    struct object *ob;
    char *from;
{
    struct sentence *s;

    ob->ref--;
    if (d_flag > 1)
	printf("Subtr ref to ob %s: %d (%s)\n", ob->name,
		      ob->ref, from);
    debug(16384, ("subtr ref to ob %s: %d (%s)\n", ob->name, ob->ref, from));
    if (ob->ref > 0)
	return;
    if (d_flag)
	printf("free_object: %s.\n", ob->name);
    if (!(ob->flags & O_DESTRUCTED)) {
	/* This is fatal, and should never happen. */
	fatal("FATAL: Object 0x%x %s ref count 0, but not destructed (from %s).\n",
	    ob, ob->name, from);
    }
    if (ob->interactive)
	fatal("Tried to free an interactive object.\n");
    /*
     * If the program is freed, then we can also free the variable
     * declarations.
     */
    if (ob->prog) {
	tot_alloc_object_size -=
	    (ob->prog->p.i.num_variables - 1) * sizeof (struct svalue) +
		sizeof (struct object);
	free_prog(ob->prog, 1);
	ob->prog = 0;
    }
    if (ob->swap_num != -1)
	remove_swap_file(ob);
    for (s = ob->sent; s;) {
	struct sentence *next;
	next = s->next;
	free_sentence(s);
	s = next;
    }
    if (ob->name) {
	if (d_flag > 1)
	    debug_message("Free object %s\n", ob->name);
	if (lookup_object_hash(ob->name) == ob)
	    fatal("Freeing object %s but name still in name table", ob->name);
	FREE(ob->name);
	ob->name = 0;
    }
    tot_alloc_object--;
    FREE((char *)ob);
}

/*
 * Allocate an empty object, and set all variables to 0. Note that a
 * 'struct object' already has space for one variable. So, if no variables
 * are needed, we allocate a space that is smaller than 'struct object'. This
 * unused (last) part must of course (and will not) be referenced.
 */
struct object *get_empty_object(num_var)
    int num_var;
{
    static struct object NULL_object;
    struct object *ob;
    int size = sizeof (struct object) +
	(num_var - !!num_var) * sizeof (struct svalue);
    int i;

    tot_alloc_object++;
    tot_alloc_object_size += size;
    ob = (struct object *)DXALLOC(size, 4096, "get_empty_object");
    /* marion
     * Don't initialize via memset, this is incorrect. E.g. the bull machines
     * have a (char *)0 which is not zero. We have structure assignment, so
     * use it.
     */
    *ob = NULL_object;
    ob->ref = 1;
    ob->swap_num = -1;
    for (i=0; i<num_var; i++)
	ob->variables[i] = const0n;
    return ob;
}

void remove_all_objects() {
    struct object *ob;
    struct svalue v;

    v.type = T_OBJECT;
    while(1) {
	if (obj_list == 0)
	    break;
	ob = obj_list;
	v.u.ob = ob;
	destruct_object(&v);
	if (ob == obj_list)
	    break;
    }
    remove_destructed_objects();
}

#ifdef CHECK_OB_REF
/*
 * For debugging purposes.
 */
void check_ob_ref(ob, from)
    struct object *ob;
    char *from;
{
    struct object *o;
    int i;

    for (o = obj_list, i=0; o; o = o->next_all) {
	if (o->inherit == ob)
	    i++;
    }
    if (i+1 > ob->ref) {
	fatal("FATAL too many references to inherited object %s (%d) from %s.\n",
	      ob->name, ob->ref, from);
	if (current_object)
	    fprintf(stderr, "current_object: %s\n", current_object->name);
	for (o = obj_list; o; o = o->next_all) {
	    if (o->inherit != ob)
		continue;
	    fprintf(stderr, "  %s\n", ob->name);
	}
    }
}
#endif /* CHECK_OB_REF */

static struct object *hashed_living[LIVING_HASH_SIZE];

static int num_living_names, num_searches = 1, search_length = 1;

static int hash_living_name(str)
    char *str;
{
    return hashstr(str, 100, LIVING_HASH_SIZE);
}

struct object *find_living_object(str, user)
    char *str;
    int user;
{
    struct object **obp, *tmp;
    struct object **hl;
    struct svalue *ret;

    if (!str) return 0;
    num_searches++;
    hl = &hashed_living[hash_living_name(str)];
    for (obp = hl; *obp; obp = &(*obp)->next_hashed_living) {
	search_length++;
	if ((*obp)->flags & O_HIDDEN)
	  {
	     push_object(current_object);
	     ret = apply_master_ob("valid_hide",1);
	     if (IS_ZERO(ret))
	       continue;
	  }
	if (user && !((*obp)->flags & O_ONCE_INTERACTIVE))
	  continue;
	if (!((*obp)->flags & O_ENABLE_COMMANDS))
	  continue;
	if (strcmp((*obp)->living_name, str) == 0)
	  break;
     }
    if (*obp == 0)
	return 0;
    /* Move the found ob first. */
    if (obp == hl)
	return *obp;
    tmp = *obp;
    *obp = tmp->next_hashed_living;
    tmp->next_hashed_living = *hl;
    *hl = tmp;
    return tmp;
}

void set_living_name(ob, str)
    struct object *ob;
    char *str;
{
    struct object **hl;

	if (ob->flags & O_DESTRUCTED)
		return;
	if (ob->living_name) {
		remove_living_name(ob);
	}
	num_living_names++;
	hl = &hashed_living[hash_living_name(str)];
	ob->next_hashed_living = *hl;
	*hl = ob;
	ob->living_name = make_shared_string(str);
	return;
}

void remove_living_name(ob)
    struct object *ob;
{
    struct object **hl;

    num_living_names--;
    if (!ob->living_name)
	fatal("remove_living_name: no living name set.\n");
    hl = &hashed_living[hash_living_name(ob->living_name)];
    while(*hl) {
	if (*hl == ob)
	    break;
	hl = &(*hl)->next_hashed_living;
    }
    if (*hl == 0)
	fatal("remove_living_name: Object named %s no in hash list.\n",
	      ob->living_name);
    *hl = ob->next_hashed_living;
    free_string(ob->living_name);
    ob->next_hashed_living = 0;
    ob->living_name = 0;
}

void stat_living_objects() {
    add_message("Hash table of living objects:\n");
    add_message("-----------------------------\n");
    add_message("%d living named objects, average search length: %4.2f\n",
		num_living_names, (double)search_length / num_searches);
}

void reference_prog (progp, from)
    struct program *progp;
    char *from;
{
    progp->p.i.ref++;
    if (d_flag)
	printf("reference_prog: %s ref %d (%s)\n",
	    progp->name, progp->p.i.ref, from);
}

/*
 * Decrement reference count for a program. If it is 0, then free the prgram.
 * The flag free_sub_strings tells if the propgram plus all used strings
 * should be freed. They normally are, except when objects are swapped,
 * as we want to be able to read the program in again from the swap area.
 * That means that strings are not swapped.
 */
void free_prog(progp, free_sub_strings)
    struct program *progp;
    int free_sub_strings;
{
    progp->p.i.ref--;
    if (progp->p.i.ref > 0)
	return;
    if (d_flag)
	printf("free_prog: %s\n", progp->name);
    if (progp->p.i.ref < 0)
	fatal("Negative ref count for prog ref.\n");
    total_prog_block_size -= progp->p.i.total_size;
    total_num_prog_blocks -= 1;
    if (free_sub_strings) {
	int i;

	/* Free all function names. */
	for (i=0; (unsigned)i < progp->p.i.num_functions; i++)
	    if (progp->p.i.functions[i].name)
		free_string(progp->p.i.functions[i].name);
	/* Free all strings */
	for (i=0; (unsigned)i < progp->p.i.num_strings; i++)
	    free_string(progp->p.i.strings[i]);
	/* Free all variable names */
	for (i=0; (unsigned)i < progp->p.i.num_variables; i++)
	    free_string(progp->p.i.variable_names[i].name);
	/* Free all inherited objects */
	for (i=0; (unsigned)i < progp->p.i.num_inherited; i++)
	    free_prog(progp->p.i.inherit[i].prog, 1);
	FREE(progp->name);
    }
    FREE((char *)progp);
}

void reset_object(ob, arg)
    struct object *ob;
    int arg;
{
    extern int current_time;

    /* Be sure to update time first ! */
    ob->next_reset = current_time + TIME_TO_RESET/2 +
	random_number(TIME_TO_RESET/2);
    if (arg == 0) {
	apply("__INIT", ob, 0);
	apply("create", ob, 0);
    } else { /* check for O_WILL_RESET in backend */
    struct object *save_command_giver;

    save_command_giver = command_giver;
	command_giver = (struct object *)0;
	if (!apply("reset", ob, 0)) { /* no reset() in the object */
		ob->flags &= ~O_WILL_RESET; /* don't call it next time */
	}
	command_giver = save_command_giver;
    }
    ob->flags |= O_RESET_STATE;
}

/*
 * If there is a shadow for this object, then the message should be
 * sent to it. But only if catch_tell() is defined. Beware that one of the
 * shadows may be the originator of the message, which means that we must
 * not send the message to that shadow, or any shadows in the linked list
 * before that shadow.
 */
#ifndef NO_SHADOWS 
int shadow_catch_message(ob, str)
    struct object *ob;
    char *str;
{
    if (!ob->shadowed)
	return 0;
    while(ob->shadowed != 0 && ob->shadowed != current_object)
	ob = ob->shadowed;
    while(ob->shadowing) {
	if (function_exists("catch_tell", ob))
	{
	    push_constant_string(str);
	    if (apply("catch_tell", ob, 1)) /* this will work, since we know the */
		/* function is defined */
		return 1;
	}
	ob = ob->shadowing;
    }
    return 0;
}
#endif

INLINE int
object_visible(ob)
struct object *ob;
{
	struct svalue *ret;

	if (ob->flags & O_HIDDEN) {
		if (current_object->flags & O_HIDDEN) {
			return 1;
		}
		push_object(current_object);
		ret = apply_master_ob("valid_hide", 1);
		return !IS_ZERO(ret);
	} else {
		return 1;
	}
}
