#include "../lpc_incl.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../backend.h"
#include "../compiler.h"
#include "../main.h"
#include "../eoperators.h"
#include "../efuns_main.h"
#include "../efun_protos.h"
#include "../simul_efun.h"
#include "../add_action.h"
#include "../array.h"
#include "../master.h"
#include "../port.h"
#include "../array.h"

#ifdef PACKAGE_PKUXKX

char* translate_easy(char* encoding, char* input)
{
  char* output = (char *)DMALLOC(strlen(input) + 1, TAG_PERMANENT, "translate");	
  strcpy(output,input);
  output[0] = '+';
  return output;
}

void f_gb_to_utf8()
{
  char *text = (char *)sp->u.string;
  char *translated = translate_easy("gbk", text);
  pop_stack();
  copy_and_push_string(translated);
	
}
void f_utf8_to_gb()
{
  char *text = (char *)sp->u.string;
  char *translated = translate_easy("gbk", text);
  pop_stack();
  copy_and_push_string(translated);
	
}
#endif

