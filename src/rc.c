/*
 * rc.c
 * description: runtime configuration for lpmud
 * author: erikkay@mit.edu
 * last modified: 2/16/92
 */

#include <stdio.h>
#include "config.h"
#include "lint.h"

#define NUM_STRS 12
#define NUM_INTS 21

static char config_str[NUM_STRS][80];
static long config_int[NUM_INTS];
static char *buff;

INLINE long get_config_int (num)
int num;
{
  if (num > NUM_INTS || num < 0) 
    {
      fatal ("Bounds error in get_config_int\n");
    }
  return config_int[num];
}

INLINE char * get_config_str (num)
     int num;
{
  if (num > NUM_STRS || num < 0) 
    {
      fatal ("Bounds error in get_config_str\n");
    }
  return config_str[num];
}

void read_config_file (file)
     FILE * file;
{
  char str[120];
  int size = 0,len;
  buff = (char *) malloc(80 * (NUM_INTS + 1) * (NUM_STRS + 1));
  while (1) 
    {
      if (fgets (str,120,file) == NULL)
	break;
      if (!str) break;
      len = strlen(str);
      if (len > 80) {
	fprintf (stderr,"*Error in config file: line too long.\n");
	exit (-1);
      }
      if (str[0] != '#' && str[0] != '\n')
	{
	  size += len + 1;
	  if (size > (80 * (NUM_INTS + 1) * (NUM_STRS + 1)))
	    buff = (char *)realloc (buff, size);
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
  char missing_line[80];

  tmp = (char *)strchr (start,fmt[0]);
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
  if (!def)
    def = fopen(filename,"r");
  if (!def) 
    {
      fprintf 
	(stderr,"*Error: couldn't load defaults file : %s\n",filename);
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
#if 0
  scan_config_line(buff,"global include file : %[^\n]",config_str[11],0);
#endif
  
  scan_config_line(buff,"time to clean up : %ld\n",&config_int[0],1);
  scan_config_line(buff,"time to swap : %ld\n",&config_int[1],1);
  scan_config_line(buff,"time to reset : %ld\n",&config_int[2],1);
  scan_config_line(buff,"allowed ed commands : %ld\n",&config_int[3],1);
  scan_config_line(buff,"evaluator stack size : %ld\n",&config_int[4],1);
  scan_config_line(buff,"compiler stack size : %ld\n",&config_int[5],1);
  scan_config_line(buff,"maximum call depth : %ld\n",&config_int[6],1);
  scan_config_line(buff,"maximum bits in a bitfield : %ld\n",&config_int[7],1);
  scan_config_line(buff,"maximum local variables : %ld\n",&config_int[8],1);
  scan_config_line(buff,"maximum evaluation cost : %ld\n",&config_int[9],1);
  scan_config_line(buff,"maximum array size : %ld\n",&config_int[10],1);
  scan_config_line(buff,"maximum players : %ld\n",&config_int[11],1);
  scan_config_line(buff,"maximum log size : %ld\n",&config_int[12],1);
  scan_config_line(buff,"maximum read file size : %ld\n",&config_int[13],1);
  scan_config_line(buff,
		   "maximum commands per heartbeat : %ld\n",&config_int[14],1);
  scan_config_line(buff,"maximum byte transfer : %ld\n",&config_int[15],1);
  scan_config_line(buff,"port number : %ld\n",&config_int[16],1);
  scan_config_line(buff,"reserved size : %ld\n",&config_int[17],1);
  scan_config_line(buff,"living hash table size : %ld\n",&config_int[18],1);
  scan_config_line(buff,"hash table size : %ld\n",&config_int[19],1);
  scan_config_line(buff,"object table size : %ld\n",&config_int[20],1);
  
  fclose (def);
}
