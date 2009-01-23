/*
 *  console.c -- implements a command-line for the driver.
 *               Separate file because this is likely to grow with time.
 *               Note that the intent is NOT to have a full scripting language,
 *               since that's, er, what LPC is.  This is to implement driver
 *               inspection that is difficult or impossible from inside the
 *               mud.  Because of this, explicit allocation of things is
 *               probably a bad idea: use automatic stack variables as much
 *               as is possible.
 *               Isaac Charles (Hamlet@Discworld, etc) -- Jul 2008
 */

#include "std.h"

#ifdef HAS_CONSOLE
#include "comm.h"
#include "object.h"
#include "dumpstat.h"

#define NAME_LEN 50

typedef struct {
  char name[NAME_LEN + 1];
  int cpy;
  int ref;
  long mem;
  long idle;
} ITEMS;

  
static int strcmpalpha(const char *aname, const char *bname, int depth);
static int objcmpalpha(const void *, const void *);
static int itemcmpsize(const void *, const void *);
static int objcmpsize(const void *, const void *);
static int objcmpidle(const void *, const void *);

static INLINE void print_obj(int refs, int cpy, const char *obname, long lastref,
                             long sz) {
  printf("%4d %4d %-*s %5ld %10ld\n", refs, cpy, NAME_LEN,
                                    obname, ((long)time(0) - lastref), sz);
}

void console_command(char *s) {
  char verb[11];
  char args[3][160];
  int numargs, num[2];

  if(strlen(s) > 0) {
    if(strncmp(s, "quit", strlen(s)) == 0) {
      (void) puts("Console closing.  The mud remains.");
      //(void) fclose(stdin);
      has_console = 0;
      restore_sigttin();
    }
    else if(strncmp(s, "shutdown", ((strlen(s) > 4) ? strlen(s) : 4)) == 0) {
      (void) puts("Mud ending now.");
      shutdownMudOS(0);
      return;
    }
    else if(strncmp(s, "help", strlen(s)) == 0) {
      (void) puts(
"Console Help\n\
    quit       Close the console (the MUD remains)\n\
    shutdown   End the mud (rapidly)\n\
    help       This.\n\
    objects    Show a listing of all active objects.  Format:\n\
               objects [qualifier] [val|range] [prefix=PREFIX]\n\
               qualifiers are:\n\
                 alpha    sort objects alphabetically\n\
                 size     sort objects by (reverse) size\n\
                 idle     sort objects by (reverse) idle time\n\
                 totals   sum together usage of all clones of an obj and\n\
                          display in reverse order of size\n\
                 depth:<D>    sum together subdirectories past depth D.\n\
               val is a number, specifying number of objects to show,\n\
               from the beginning (positive) or from the end (negative).\n\
               range is st..fin where st and fin can be number of objects\n\
                from the beginning (positive) or from the end (negative.)\n\
               if prefix=PREFIX is present, only items beginning with PREFIX\n\
                will be displayed.  Note that ranges are applied and /then/ \n\
                the prefixes are checked.  Specifying both may result in \n\
                empty output\
");
    }
    else { // commands that can be followed by args
      int DEPTH = 1000; /* allow summarization to dir-level */
      char *prefix = (char *) 0;
      int plen = 0;

      num[0] = 0;
      num[1] = INT_MAX;

      if((numargs = sscanf(s, "%10s %159s %159s %159s", verb, args[0], 
                           args[1], args[2]) - 1) 
         >= 1) {
        int tmp;
        char *pos;

        if(numargs && (strncmp("prefix", args[numargs-1], 6) == 0) &&
           (strlen(args[numargs-1]) > 6)) {
          prefix = args[numargs-1] + 7;
          plen = strlen(prefix);
          numargs--;
        }
        
        if(numargs) {
          pos = args[numargs-1];
        while((pos = strchr(pos, '<')) != NULL)
          *pos++ = '-';

          if((tmp = sscanf(args[numargs-1], "%d..%d", &num[0], &num[1])) == 2){
        }
        else if(tmp == 1) {
          if(args[numargs-1][strlen(args[numargs-1])-1] != '.') {
            if(num[0] == 0) {
              num[0] = num[1] = 0;
            }
            else if(num[0] > 0) {
              num[1] = num[0] - 1;
              num[0] = 0;
            }
          }
        }

        if(tmp)
          numargs--;
      }
      }

      if(strncmp(verb, "objects", strlen(verb)) == 0) {
        object_t **list;
        int count;
        char *pos2;
        int i;
        long totsz = 0, sz;
        char TOTALS = (char) 0;

        /* allow 'objects depth:3 10' */
        if(numargs && ((pos2 = strpbrk(args[0], ":=")) != NULL)) {
          sscanf(pos2+1, "%d", &DEPTH);
          if(DEPTH < 1)
            DEPTH = 1;
          *pos2 = '\0';
        }
        
        // get_objects() allocates a string and pushes it onto the stack.
        // probably not a problem here as long as we remember to pop it.
        get_objects(&list, &count, 0, 0);

        if(numargs) {
          if(strncmp("alpha", args[0], strlen(args[0])) == 0)
            qsort(list, (size_t) count, sizeof(object_t *), objcmpalpha);
          else if(strncmp("size", args[0], strlen(args[0])) == 0)
            qsort(list, (size_t) count, sizeof(object_t *), objcmpsize);
          else if(strncmp("idle", args[0], strlen(args[0])) == 0)
            qsort(list, (size_t) count, sizeof(object_t *), objcmpidle);
          else { /* This should be "totals" or "depth" */
            TOTALS = (char) 1;
            qsort(list, (size_t) count, sizeof(object_t *), objcmpalpha);
          }
        }

        printf("%4s %4s %-*s %5s %10s\n", "Rf", "Cpy", NAME_LEN, "Obj",
                                          "Idle", "Size");

        if(TOTALS && (count > 1)) {
          ITEMS it[count+1];
          int lst = 0;
          char *pound;
          //char tmpnam[NAME_LEN + 1];

          memset(it, 0, (count+1) * sizeof(ITEMS));

          for(i=0; i < count; i++) {
            if(strcmpalpha(it[lst].name, list[i]->obname, DEPTH)
               != 0) {
              int sl;
              const char *slash = list[i]->obname;
              lst++;

              for(sl = 0; sl < DEPTH; sl++)
                if((slash = strchr(slash, '/')) == NULL) {
                  slash = list[i]->obname + strlen(list[i]->obname);
                  break;
                }
                else
                  slash++;
                
              strncpy(it[lst].name, list[i]->obname +
                                    (((slash - list[i]->obname) > NAME_LEN) ?
                                     ((slash - list[i]->obname) - NAME_LEN) 
                                      : 0)
                                  , NAME_LEN);
              it[lst].name[slash - list[i]->obname] = '\0';

              pound = strchr(it[lst].name, '#');
              if(pound != NULL)
                *pound = '\0';
            }

            it[lst].cpy++;
            it[lst].ref += list[i]->ref;
            it[lst].mem += list[i]->prog->total_size + data_size(list[i]) +
                           sizeof(object_t);
            if(((long)list[i]->time_of_ref < it[lst].idle) || !it[lst].idle)
              it[lst].idle = (long) list[i]->time_of_ref;
          }

          qsort(it+1, (size_t) lst, sizeof(ITEMS), itemcmpsize);

          if(num[0] >= lst)
            num[0] = lst - 1;
          else if(num[0] < 0) {
            num[0] = lst + num[0];
            if(num[0] < 0)
              num[0] = 0;
          }
          num[0]++;

          if(num[1] >= lst)
            num[1] = lst - 1;
          else if(num[1] < 0) {
            num[1] = lst + num[1];
            if(num[1] < 0)
              num[1] = 0;
          }
          num[1]++;

          for(i=num[0]; i<=num[1]; i++) {
            if(!prefix || (strncmp(it[i].name, prefix, plen) == 0)) {
            print_obj(it[i].ref, it[i].cpy, it[i].name, it[i].idle,
                       it[i].mem);
            totsz += it[i].mem;
          }
        }
        }
        else { // no totals needed
          if(num[0] >= count)
            num[0] = count - 1;
          else if(num[0] < 0) {
            num[0] = count + num[0];
            if(num[0] < 0)
              num[0] = 0;
          }
          if(num[1] >= count)
            num[1] = count - 1;
          else if(num[1] < 0) {
            num[1] = count + num[1];
            if(num[1] < 0)
              num[1] = 0;
          }

          for(i = num[0]; i <= num[1]; i++) {
            if(!prefix || (strncmp(list[i]->obname, prefix, plen) == 0)) {
            print_obj((int)list[i]->ref, 1, list[i]->obname,
                      (long) list[i]->time_of_ref,
                      sz = (long) list[i]->prog->total_size +
                           (long)data_size(list[i]) +
                           (long)sizeof(object_t));
            totsz += sz;
          }
        }
        }

        printf("%4s %4s %-*s %5s %10s\n", "", "", NAME_LEN, "-----", "-----",
                                           "----------");
        printf("%4s %4s %*d %-5s %10ld\n", "", "", NAME_LEN, 1+num[1]-num[0],
                                          "objs", totsz);
        pop_stack();
      }
      else {
        (void) puts("Unknown command.");
      }
    }
  }

  if(has_console > 0) {
    (void) putchar('?'); (void) putchar(' ');
    (void) fflush(stdout);
  }
}

static int strcmpalpha(const char *aname, const char *bname, int depth) {
  int slash = 0;

  for(;(*aname == *bname) && (*aname != '#') && (*bname != '#') &&
       (*aname != '\0') && (*bname != '\0'); aname++, bname++) {
    if(*aname == '/')
      slash++;
    
    if(slash == depth)
      break;
  }

  if((*aname == '#') || ((slash == depth) && (*aname == '/'))) {
    if((*bname != '#') && (*bname != '\0') && 
       ((slash != depth) || (*bname != '/')))
      return 1;
    return 0;
  }
  if((*bname == '#') || ((slash == depth) && (*bname == '/'))) {
    if(*aname == '\0')
      return 0;
    return -1;
  }

  return (int) (*aname - *bname);
}

static int objcmpalpha(const void *a, const void *b) {
  const char *aname = (*((object_t **) a))->obname;
  const char *bname = (*((object_t **) b))->obname;

  return strcmpalpha(aname, bname, 1000);
}

static int itemcmpsize(const void *a, const void *b) {
  long asz = (*((ITEMS *) a)).mem;
  long bsz = (*((ITEMS *) b)).mem;

  if(asz == bsz)
    return strcmpalpha((*((ITEMS *) a)).name, (*((ITEMS *) b)).name, 1000);

  return (int) (bsz - asz);
}

static int objcmpsize(const void *a, const void *b) {
  object_t *aobj = (*((object_t **) a));
  object_t *bobj = (*((object_t **) b));
  int asz = aobj->prog->total_size + data_size(aobj);
  int bsz = bobj->prog->total_size + data_size(bobj);

  if(asz == bsz)
    return objcmpalpha(a, b);

  return bsz - asz;
}

static int objcmpidle(const void *a, const void *b) {
  int arft = (*((object_t **) a))->time_of_ref;
  int brft = (*((object_t **) b))->time_of_ref;

  if(arft == brft)
    return objcmpalpha(a, b);

  return arft - brft;
}
#endif
