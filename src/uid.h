/*
 * uid.h
 * Created by: Erik Kay
 * Description: new uid / euid data structures and security
 * Modification:
 *    11-1-92 - Erik Kay - initial creation
 */

typedef struct _uid {
  char *name;
  struct _uid *next;
} userid_t;


userid_t *find_uid PROT((char *name));
void insert_uid PROT((userid_t *new_uid));
userid_t *add_uid PROT((char *name));
userid_t *set_root_uid PROT((char *name));
userid_t *set_backbone_uid PROT((char *name));

