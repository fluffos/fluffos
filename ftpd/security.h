#define PORT 3001
#define MUD_PATH "/home/couns/sstock/src/mos/lib"
#define FTPD_LOG "/home/couns/sstock/src/mos/ftpd.log"

struct mudpw {
	char pw_name[14];
	char pw_passwd[26];
	char pw_position[20];
	int pw_level;
	char pw_dir[256];
};

/* undef ALLOW_GUEST to prevent anonymous ftp */
#undef ALLOW_GUEST
