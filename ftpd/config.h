/* The following defines are used for setting up your own LPmud
 * Server.
 */

/* Define this to be the port number for your wizards to ftp to.
 * For example : ftp 129.25.7.111 <port>
 */

#define PORT 2998


/* Define this to be the absolute pathname to your mudlib directory.
 */

#define MUD_PATH "/home/couns/sstock/src/mos/lib"

/* Wizards home directories?
 */

#define WIZARD_DIR "wizards"

/* If you have domains please define the following things according to
 * domains access.
 */

#define DOMAINS           /* undef if you do not have domains */

#define DOMAIN_DIR "d"


/* Define this to be where you wish your log files to appear.
 */

#define FTPD_LOG "/home/couns/sstock/src/mos/ftpd.log"


/* Define the undef below if you wish to allow anonymous ftp logins.
 * Login name can be "ftp", "anonymous" or "guest".
 */

#define ALLOW_GUEST
#undef ALLOW_GUEST


/*_____________________________________________________________________________
 *NO NEED TO REDEFINE ANYTHING BELOW HERE UNLESS YOU ARE CHANGING THE FTPD CODE
 *_____________________________________________________________________________
 */

struct mudpw {
	char pw_name[14];
	char pw_passwd[26];
	int pw_level;
        int pw_access;
	char pw_dir[256];
};

/* The defines below are the allowed defines in /secure/FTP_ACCESS
 */

#define ROOT		0
#define ELDER		1
#define LORD		2
#define WIZARD		3
#define RESTRICTED	4
