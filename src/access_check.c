#include "config.h"

#ifdef ACCESS_RESTRICTED	/* Comment out whole file */

#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>

#include "lint.h"

/* check a given internet address against patterns given in an ACCESS.ALLOW
 * file.                       
 * Written by Markus Wild for Loch Ness in 1991.
 * Spread by gec with permission of Markus Wild. 
 * Source is in the public domain. No charges allowed.
 *
 * notice the time stamp when we last scanned the file, and rescan if it
 * changed since that (this enables us to edit the file without having to
 * reboot lpmud to read it)
 * NOTICE: when changing the access file, the old tables ARE DISCARDED. This
 *         means, that users logged in will not count for the per-class
 *         maximum. This will normalize, as soon as users log out, as the
 *         "currently" counter will not go below 0.
 */


extern char *inet_ntoa(), *strtok();

/* simply remove this line if you wish access to your system to be logged */
#undef ACCESS_LOG

/* maximal string length to be output */
#define MAX_MESSAGE 255

struct access_class {
  int class_num;		/* the class number */
  int currently;		/* currently <= this number of logged in users */
  int maximum;			/* 0: disallowed, -1: no maximum */
  char message[MAX_MESSAGE];	/* message to be printed in case a login can't be permitted */
};

struct access_address {
  int addr[4];		/* [0..255]: number, -1: all */
  int hstart, hend;	/* start/end hour */
  struct access_class *ac;
};

static struct access_address *addr_tab = 0;
static int addr_tab_index, addr_tab_size;
static struct access_class *class_tab = 0;
static int class_tab_index, class_tab_size;
static time_t last_read = 0;

static void log_access ();

/* check the file, and if it was changed, (re)read it into memory */
static void
check_read_file (name)
     char *name;
{
  struct stat stb;
  FILE *in;

  if (!(stat (name, &stb)))
    {
      if (stb.st_mtime > last_read)
	{
	  /* throw away the old information */
	  if (addr_tab) FREE((char *)addr_tab), addr_tab = 0;
	  if (class_tab) FREE((char *)class_tab), class_tab = 0;
	  addr_tab_size = 10;
	  addr_tab_index = 0;
	  addr_tab = (struct access_address *)
	      DMALLOC(addr_tab_size * sizeof (struct access_address),
			122, "check_read_file: addr_tab");
	  class_tab_size = 10;
	  class_tab_index = 0;
	  class_tab = (struct access_class *)
	      DMALLOC(class_tab_size * sizeof (struct access_class),
			1, "check_read_file: class_tab");
	  
	  if (in = fopen (name, "r"))
	    {
	      while (!feof (in))
		{
		  char buffer [2*MAX_MESSAGE]; /* heuristic ;-)) */
		  char addr1[4], addr2[4], addr3[4], addr4[4];
		  struct access_address aa;
		  struct access_class ac;
		  char *cp;
		  int i;

		  if (! fgets (buffer, 2*MAX_MESSAGE, in)) break;
		  if (buffer[0] == '#') continue; /* a comment, skip */
		  /* if there is no ':' in there, this is probably an empty line */
		  if (! strchr (buffer, ':')) continue;
		  
		  /* more or less no error-checking ;-)) */
		  strncpy (addr1, strtok (buffer, "."), 3);
		  strncpy (addr2, strtok (0, "."), 3);
		  strncpy (addr3, strtok (0, "."), 3);
		  strncpy (addr4, strtok (0, ":"), 3);

		  ac.class_num = atoi (strtok (0, ":"));
		  ac.currently = 0;
		  ac.maximum = atoi (((cp = strtok (0, ":")), (cp && *cp) ? cp : "0"));
		  aa.hstart = atoi (((cp = strtok (0, ":")), (cp && *cp) ? cp : "0"));
		  aa.hend   = atoi (((cp = strtok (0, ":")), (cp && *cp) ? cp : "0"));
		  strncpy (ac.message,
			   (cp = strtok (0, "\n")) ? cp : "", MAX_MESSAGE-1);

		  /* check whether this class is already defined */
		  for (i = 0; i < class_tab_index; i++)
		    if (class_tab[i].class_num == ac.class_num)
		      {
			/* in this case just set a pointer to the defined class */
			aa.ac = &class_tab[i];
			break;
		      }

		  /* if not, define it */
		  if (i == class_tab_index)
		    {
		      class_tab[class_tab_index] = ac;
		      if (++class_tab_index == class_tab_size)
			{
			  class_tab_size <<= 1;
			  class_tab = (struct access_class *)
			      DREALLOC((char *)class_tab,
				       class_tab_size * sizeof (struct access_class),
						2, "check_read_file: class_tab: realloc");
			}

		      aa.ac = &class_tab[i];
		    }

		  /* now set up the address, * maps into -1, anything else is vanilla */
		  aa.addr[0] = strcmp(addr1, "*") ? atoi (addr1) : -1;
		  aa.addr[1] = strcmp(addr2, "*") ? atoi (addr2) : -1;
		  aa.addr[2] = strcmp(addr3, "*") ? atoi (addr3) : -1;
		  aa.addr[3] = strcmp(addr4, "*") ? atoi (addr4) : -1;

		  /* and add it to our address table */
		  addr_tab[addr_tab_index] = aa;
		  if (++addr_tab_index == addr_tab_size)
		    {
		      addr_tab_size <<= 1;
		      addr_tab = (struct access_address *)
			  DREALLOC((char *)addr_tab,
				   addr_tab_size * sizeof (struct access_address),
					3, "check_read_file: addr_tab: realloc");
		    }
		} /* over total input */
	      fclose (in);
	      last_read = stb.st_mtime;
	      return;
	    } /* if open succeeded */
	}
    }
	else
		fprintf(stderr,"Couldn't open %s for reading.\n",name);
}

/* the main function, validate an address (peer of given socket).
 * return 0, if access is not permitted, else return a pointer to the
 * corresponding class. Pass that pointer on logout to "release_host_access".
 */
struct access_class *
allow_host_access (sockfd, outfd)
     int sockfd, outfd;
{
  struct sockaddr_in apa;
  int addr[4];
  int len;
  char *ipname, *fname;
  struct access_address *ap;
  int i;
#define STRING(str) str,strlen(str)
  
  fname = (char *)DMALLOC(strlen(ACCESS_FILE) + 1, 4,
	"allow_host_access: fname");
  sprintf (fname,"%s",ACCESS_FILE);
  if (fname[0] == '/')
    strcpy (fname,fname+1);
  check_read_file (fname);
  FREE(fname);

  len = sizeof (apa);
  if (getpeername (sockfd, (struct sockaddr *)&apa, &len) == -1)
    {
      perror ("getpeername");
      write (outfd, STRING ("Sorry, internal MudOS error.\n"));
      return 0;
    }
#if !defined(NeXT) && !defined(__SEQUENT__)
  ipname = inet_ntoa(apa.sin_addr);
#else
  ipname = inet_ntoa(ntohl(apa.sin_addr));
#endif
  sscanf (ipname, "%d.%d.%d.%d", addr, addr + 1, addr + 2, addr + 3);

  for (i = 0, ap = addr_tab; i < addr_tab_index; i++, ap++)
    {
      int pos;
      /* check for address. match if either equal or wildcard */
      for (pos = 0; pos < 4; pos++)
	if (ap->addr[pos] != addr[pos] && ap->addr[pos] != -1) break;

      if (pos == 4) /* a match */
	{
	  /* if hstart and hend are not == 0, check whether ap is in the
	   * interval */
	  if (ap->hstart || ap->hend)
	    {
	      time_t now = time(0);
	      struct tm *tm = localtime(&now);
	      if (ap->hstart < ap->hend)
		{
		  if (tm->tm_hour < ap->hstart || tm->tm_hour > ap->hend)
		    continue;
		}
	      else
		{
		  if (tm->tm_hour > ap->hend && tm->tm_hour < ap->hstart)
		    continue;
		}
	    }

	  /* no maxmium? */
	  if (ap->ac->maximum == -1) 
            { /*gc*/
              log_access (ipname, 1);
              return ap->ac;
            }
	  /* else there is a maximum, in the worst case 0 */
	  if (ap->ac->currently >= ap->ac->maximum)
	    {
	      write (outfd, ap->ac->message, strlen (ap->ac->message));
	      write (outfd, "\n", 1);
	      shutdown (outfd, 2); close (outfd);
	      log_access (ipname, 0);
	      return 0;
	    }
	  /* bump up the counter */
	  ap->ac->currently ++;
	  log_access (ipname, 1);
	  return ap->ac;
	}
    }

  /* default is: don't allow access */
  write (outfd, STRING("Sorry, you're not allowed to use this LPmud.\n"));
  shutdown (outfd, 2); close (outfd);
  log_access (ipname, 0);
  return 0;
}

/* decrement the currently counter once. This is called, when a user loggs out. */

void
release_host_access (class)
     struct access_class *class;
{
  if (class->maximum != -1 && class->currently > 0)
    -- class->currently;
}

static void
log_access (addr, ok)
     char *addr;
     int ok;
{
#ifdef ACCESS_LOG
  char *fname;
  FILE *log;

  fname = DMALLOC(strlen(ACCESS_LOG)+1, 5, "log_access: fname");
  sprintf (fname,"%s",ACCESS_LOG);
  if (fname[0] == '/')
    strcpy (fname,fname+1);
  log = fopen (fname, "a");

  if (log)
    {
      fprintf (log, "%s: %s\n", addr,  ok ? "granted" : "denied");
      fclose (log);
    }
#endif
}

#endif /* ACCESS_RESTRICTED */
