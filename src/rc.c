/*
 * rc.c
 * description: runtime configuration for lpmud
 * author: erikkay@mit.edu
 * last modified: 2/16/92
 */

#include <stdio.h>
#include "config.h"
#include "lint.h"

#define NUM_STRS 14
#define NUM_INTS 25

#define MAX_LINE_LENGTH 120

static char config_str[NUM_STRS][MAX_LINE_LENGTH];
static int config_int[NUM_INTS];
static char *buff;

INLINE int get_config_int (num)
int num;
{
#ifdef DEBUG
  if (num > NUM_INTS || num < 0) 
    {
      fatal ("Bounds error in get_config_int\n");
    }
#endif
  return config_int[num];
}

INLINE char * get_config_str (num)
     int num;
{
#ifdef DEBUG
  if (num > NUM_STRS || num < 0) 
    {
      fatal ("Bounds error in get_config_str\n");
    }
#endif
  return config_str[num];
}

void read_config_file (file)
     FILE * file;
{
  char str[120];
  int size = 0,len;
  buff = (char *)
	DMALLOC(MAX_LINE_LENGTH * (NUM_INTS + 1) * (NUM_STRS + 1), 92,
		"read_config_file: 1");
  strcat(buff,"\n");
  while (1) 
    {
      if (fgets (str,120,file) == NULL)
	break;
      if (!str) break;
      len = strlen(str);
      if (len > MAX_LINE_LENGTH) {
	fprintf (stderr,"*Error in config file: line too long.\n");
	exit (-1);
      }
      if (str[0] != '#' && str[0] != '\n')
	{
	  size += len + 1;
	  if (size > (MAX_LINE_LENGTH * (NUM_INTS + 1) * (NUM_STRS + 1)))
	    buff = (char *)DREALLOC(buff, size, 93, "read_config_file: 2");
	  strcat (buff, str);
	  strcat (buff, "\n");
	}
    }
}


/*
 * If the required flag is 0, it will only give a warning if the line is
 * missing from the config file.  Otherwise, it will give an error and exit
 * if the line isn't there.
 */
void scan_config_line (start, fmt, dest, required)
     char *start;
     char *fmt;
     void *dest;
     int required;
{
  char *tmp;
  char missing_line[MAX_LINE_LENGTH];

  tmp = (char *)strchr (start,fmt[0]);
  if (tmp && (*(tmp-1) != '\n'))
    {
      scan_config_line (tmp+1,fmt,dest, required);
      return;
    }
  if (!tmp) 
    {
      strcpy (missing_line,fmt);
      tmp = (char *)strchr (missing_line,':');
      *tmp = '\0';
      if (!required)
	{
	  fprintf (stderr,"*Warning: Missing line in config file:\n\t%s\n",
		   missing_line);
	  memset(dest,0,1);
	  return;
	}
      fprintf (stderr,"*Error in config file.  Missing line:\n\t%s\n",
	       missing_line);
      exit(-1);
    }
  if (sscanf (tmp,fmt,dest) != 1)
    scan_config_line (tmp+1,fmt,dest, required);
}

void set_defaults (filename) 
     char * filename;
{
  FILE *def;
  char defaults[SMALL_STRING_SIZE];
  
  sprintf(defaults,"%s/%s",CONFIG_FILE_DIR,filename);
  def = fopen (defaults,"r");
  if (def) {
      fprintf(stderr,"loading config file: %s\n", defaults);
  }
  else {
    def = fopen(filename,"r");
    if (def) {
      fprintf(stderr,"loading config file: %s\n", filename);
    }
  }
  if (!def) 
    {
      fprintf(stderr,"*Error: couldn't load config file: '%s'\n",filename);
      exit (-1);
    }
  read_config_file (def);
  
  scan_config_line(buff,"name : %[^\n]",config_str[0],1);
  scan_config_line(buff,"mudlib directory : %[^\n]",config_str[1],1);
  scan_config_line(buff,"binary directory : %[^\n]",config_str[2],1);
  scan_config_line(buff,"swap file : %[^\n]",config_str[3],1);
  scan_config_line(buff,"config directory : %[^\n]",config_str[4],1);
  scan_config_line(buff,"log directory : %[^\n]",config_str[5],1);
  scan_config_line(buff,"master file : %[^\n]",config_str[6],1);
  scan_config_line(buff,"access allow file : %[^\n]",config_str[7],1);
  scan_config_line(buff,"access log file : %[^\n]",config_str[8],1);
  scan_config_line(buff,"include directories : %[^\n]",config_str[9],1);
  scan_config_line(buff,"simulated efun file : %[^\n]",config_str[10],1);
  scan_config_line(buff,"address server ip : %[^\n]",config_str[11],1);
  scan_config_line(buff,"default error message : %[^\n]",config_str[12],0);
  scan_config_line(buff,"default fail message : %[^\n]",config_str[13],0);
  
  scan_config_line(buff,"time to clean up : %d\n",&config_int[0],1);
  scan_config_line(buff,"time to swap : %d\n",&config_int[1],1);
  scan_config_line(buff,"time to reset : %d\n",&config_int[2],1);
  scan_config_line(buff,"allowed ed commands : %d\n",&config_int[3],1);
  scan_config_line(buff,"evaluator stack size : %d\n",&config_int[4],0);
  scan_config_line(buff,"compiler stack size : %d\n",&config_int[5],0);
  scan_config_line(buff,"maximum call depth : %d\n",&config_int[6],1);
  scan_config_line
    (buff,"maximum bits in a bitfield : %d\n",&config_int[7],1);
  scan_config_line(buff,"maximum local variables : %d\n",&config_int[8],0);
  scan_config_line(buff,"maximum evaluation cost : %d\n",&config_int[9],1);
  scan_config_line(buff,"maximum array size : %d\n",&config_int[10],1);
  scan_config_line(buff,"maximum mapping size : %d\n",&config_int[11],1);
  scan_config_line(buff,"maximum users : %d\n",&config_int[12],0);
  scan_config_line(buff,"maximum log size : %d\n",&config_int[13],1);
  scan_config_line(buff,"maximum read file size : %d\n",&config_int[14],1);
  scan_config_line(buff,"maximum string length : %d\n",&config_int[15], 1);
  scan_config_line(buff,"address server port : %d\n",&config_int[16],1);
  scan_config_line(buff,"maximum byte transfer : %d\n",&config_int[17],1);
  scan_config_line(buff,"port number : %d\n",&config_int[18],1);
  scan_config_line(buff,"reserved size : %d\n",&config_int[19],1);
  scan_config_line(buff,"living hash table size : %d\n",&config_int[20],0);
  scan_config_line(buff,"hash table size : %d\n",&config_int[21],1);
  scan_config_line(buff,"object table size : %d\n",&config_int[22],1);
  scan_config_line(buff,"inherit chain size : %d\n",&config_int[23],1);
#if 0 /* not yet used */
  scan_config_line(buff,"maximum efun sockets : %d\n",&config_int[24],0);
#endif
  FREE(buff); 
  fclose (def);
}
