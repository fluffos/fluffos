#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

extern struct mudpw *pw;
extern int guest;        /* anonymous guest or not */

int anonymous(s)
char *s;
{
#ifdef ALLOW_GUEST
      return (!strcmp(s,"ftp") || !strcmp(s,"anonymous") || !strcmp(s,"guest"));
#else
      return 0;
#endif
}

/* This is where the FTP_ACCESS file which should be in /secure in your
 * mudlib will decide access for the user logged in.
 */

int access_allowed(pw)
struct mudpw *pw;
{
	FILE *access;
	char s[256];
        char a[50], b[50];
        int i;

	sprintf(s, "%s/adm/etc/FTP_ACCESS", MUD_PATH);
	if ((access = fopen(s, "r")) == NULL)
          return WIZARD;
	while (fgets(s, 255, access) != NULL) {
             if (s[0] == '#') continue;
             for (i = 0; s[i] != '\0'; i++) {
		if (isalpha(s[i]))
		  s[i] = tolower(s[i]);
             }
             if (sscanf(s, "%[^::]::%s", a, b) == 2) {
               if (strncmp(a, pw->pw_name, strlen(pw->pw_name)) == 0) {
		 if (strcmp(b, "root") == 0) return ROOT;
                 if (strcmp(b, "elder") == 0) return ELDER;
                 if (strcmp(b, "lord") == 0) return LORD;
                 if (strcmp(b, "wizard") == 0) return WIZARD;
		 return RESTRICTED;
               }
             }
        }
        return WIZARD;
}

/* get user's home directory */

char *get_home(pw,s)
struct mudpw *pw;
char *s;
{
	sprintf(s,"%s/%s/%s",MUD_PATH, WIZARD_DIR, pw->pw_name);
	return s;
}

/* return the filename for saved player object (the *.o file) */

char *get_player_fname(pw,s)
struct mudpw *pw;
char *s;
{
	sprintf(s,"%s/data/users/%c/%s.o",MUD_PATH,pw->pw_name[0],pw->pw_name);
	return s;
}

/*
   valid_write():

   This particular function is very dependent upon the directory structure
   of your mudlib.  Be very careful that this function is doing what you
   expect it to (restricting the appropriate directories etc - including
   the mail directory and other supposedly secure directories).
*/

/*
   This function will now check for appropriate writes in domains and according
   to the /secure/FTP_ACCESS file where you can set up special access for
   all wizards, including RESTRICTED.
*/

int valid_write(path)
char *path;
{
#ifdef DOMAINS
        FILE *members;
        char residue[MAXPATHLEN + 1];
        char dir[MAXPATHLEN + 1];
        char s[256];
        char where[MAXPATHLEN + 1];
#endif
	register int len;
        int i;
        char *path2;
	char a[MAXPATHLEN + 1];
	char b[MAXPATHLEN + 1];
	extern int errno;

	errno = 13;
	len = strlen(MUD_PATH);
	if (strncmp(path, MUD_PATH, len))
		return 0;
	path += len;
        if (pw->pw_access == ROOT) return 1;
	if (!strncmp(path,"/ftp",4))
		return 1;
	if (guest)
		return 0;
#ifdef DOMAINS
        if (sscanf(path, "/%s", where) == 1) {
          if (strncmp(where, DOMAIN_DIR, strlen(DOMAIN_DIR)) == 0)
            if (sscanf(path, "/%[^/]/%s", where, dir) == 2) {
              if (sscanf(path, "/%[^/]/%[^/]/%s", where, dir, a) != 3)
                if (sscanf(path, "/%[^/]/%s", where, dir) != 2)
                  return 0;
              sprintf(s,"%s/%s/%s/MEMBERS",MUD_PATH, DOMAIN_DIR, dir);
              if ((members = fopen(s, "r")) == NULL)
                return 0;
              i = 1;
              while (fgets(s, 255, members) != NULL) {
                   if (strncmp(s, pw->pw_name, strlen(pw->pw_name)) == 0) {
                     if (i == 2) return 1; 
                     if (strncmp(where, "common/", 7) == 0)
                       if (sscanf(where, "common/%[^/]/%s", dir, residue) == 2)
                         if (strcmp(dir, pw->pw_name) == 0)
                           return 1;
                   }
              }
              i++;
              fclose(members);
            } else
              return 0;
        }
#endif
        if (sscanf(path, "/%[^/]/%s", a, b) == 2) {
          if (strncmp(a, "open", 4) == 0) return 1;
          if (strncmp(a, WIZARD_DIR, strlen(WIZARD_DIR)) == 0)
            if ((strncmp(b, pw->pw_name, strlen(pw->pw_name)) == 0) ||
                (pw->pw_access == ELDER))
              return 1;
            else
              return 0;
        }
	return 0;
}

/*
   valid_read():

   this particular function is very dependent upon the directory structure
   of your mudlib.  Be very careful that this function is doing what you
   expect it to (restricting the appropriate directories etc - including
   the mail directory and other supposedly secure directories).

   This function has been setup now for specific access dependent apon level
   and domain membership, along with access in /secure/FTP_ACCESS
*/

int valid_read(path)
char *path;
{
#ifdef DOMAINS
        FILE *members;
        char *dir;
        char s[256];
        char where[MAXPATHLEN + 1];
#endif
	register int len;
	char a[MAXPATHLEN + 1];
	char b[MAXPATHLEN + 1];
	extern int errno;

	errno = 13;
	len = strlen(MUD_PATH);
	if (strncmp(path, MUD_PATH, len))
		return 0;
	path += len;
	if (pw->pw_access == ROOT) return 1;
	if (valid_write(path))
		return 1;
		if (!strncmp(path,"/ftp",4) || !strncmp(path,"/open",5) ||
                    !strncmp(path, "/std",4) || !strncmp(path, "/obj",4) ||
                    !strncmp(path, "/doc",4) || !strncmp(path, "/global",7))
			return 1;
		else
	if (guest) {
			return 0;
	}
#ifdef DOMAINS
        if (sscanf(path, "/%s", where) == 1) { 
          if (strncmp(where, DOMAIN_DIR, strlen(DOMAIN_DIR)) == 0) {
            if (pw->pw_access == ELDER) return 1;
            if (sscanf(path, "/%[^/]/%s", where, dir) == 2) {
              if (sscanf(path, "/%[^/]/%[^/]/%s", where, dir, a) != 3)
                if (sscanf(path, "/%[^/]/%s", where, dir) != 2)
                  return 0;
              sprintf(s,"%s/%s/%s/MEMBERS",MUD_PATH, DOMAIN_DIR, dir);
              if ((members = fopen(s, "r")) == NULL)
                return 0;
              while (fgets(s, 255, members) != NULL) {
                   if (strncmp(s, pw->pw_name, strlen(pw->pw_name)) == 0)
                     return 1;                 
              }
              return 0;
            }
          }
        }
#endif
        if (sscanf(path, "/%s", a) == 1) {
        /* Prevent people from reading "mail", "players.o's" and "secure" */
          if (strncmp(a, "save", 4) == 0) return 0;
          if (strncmp(a, "players", 7) == 0) return 0;
          if (strncmp(a, "secure", 6) == 0) return 0;
	  if (strncmp(a, "doc", 3) == 0) return 1;
        /* Protect all wizards directories!!!!!!  */
          if (strncmp(a, WIZARD_DIR, strlen(WIZARD_DIR)) == 0) {
            if (sscanf(path, "/%[^/]/%s", a, b) == 2) {
              if ((strncmp(b, pw->pw_name, strlen(pw->pw_name)) == 0) ||
                  (pw->pw_access == LORD))
                return 1;
              else
                return 0;
            }
          }
        }
        return 1;
}
