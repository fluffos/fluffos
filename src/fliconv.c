#include "fliconv.h"
#include <errno.h>
#include "std.h"
#include "lpc_incl.h"
#include "comm.h"

#ifdef USE_ICONV

struct translation *head;

static struct translation *find_translator(char *encoding){
    struct translation *cur = head;
    while(cur){
	if(!strcmp(cur->name, encoding))
	    break;
	cur = cur->next;
    }
    return cur;
}


struct translation *get_translator(char *encoding){
    struct translation *ret = find_translator(encoding);
    if(ret)
	return ret;
    ret = MALLOC(sizeof(struct translation));
    char *name = MALLOC(strlen(encoding)+18+1);
    strcpy(name, encoding);
#ifdef linux
    strcat(name, "//TRANSLIT//IGNORE");
#endif
    ret->name = name;
    ret->incoming = iconv_open("UTF-8", encoding);
    ret->outgoing = iconv_open(name, "UTF-8");
    
    ret->next = 0;
    if(ret->incoming == (iconv_t)-1 || ret->outgoing == (iconv_t)-1){
	FREE(name);
	FREE(ret);
	return 0;
    }
    name[strlen(encoding)] = 0;
    if(!head)
	head = ret;
    else {
	struct translation *cur = head;
	while(cur->next)
	    cur = cur->next;
	cur->next = ret;
    }
    return ret;
}

char *translate(iconv_t tr, const char *mes, int inlen, int *outlen){
    size_t len = inlen;
    size_t len2;
    unsigned char *tmp = (unsigned char *)mes;
    static char *res = 0;
    static size_t reslen = 0;
    char *tmp2;

    if(!res){
	res = MALLOC(1);
	reslen = 1;
    }
 
    tmp2 = res;
    len2 = reslen;

    while(len){
      iconv(tr, (char **)&tmp, &len, &tmp2, &len2);
	if(len > 1) 
#ifdef PACKAGE_DWLIB
	    if(tmp[0] == 0xff && tmp[1] == 0xf9){
		len -=2;
		tmp +=2;
#else
		if(0){
#endif
	    } else {

		if(E2BIG == errno){
		  tmp = (char *)mes;
		    len = strlen(mes)+1;
		    FREE(res);
		    reslen *= 2;
		    res = MALLOC(reslen);
		    tmp2 = res;
		    len2 = reslen;
		    continue;
		}
		tmp2[0] = 0;
		*outlen = reslen - len2;
		return res;
	    }
    }
    *outlen = reslen - len2;
    return res;
}

#else
char *translate(iconv_t tr, const char *mes, int inlen, int *outlen){
  *outlen = inlen;
  return (char *)mes;
}
#endif



char *translate_easy(iconv_t tr, char *mes){
  int dummy;
  char *res = translate(tr, mes, strlen(mes)+1, &dummy);
  return res;
}



#ifdef F_SET_ENCODING
void f_set_encoding(){
  if(current_object->interactive){
    struct translation *newt = get_translator((char *)sp->u.string);
    if(newt){
      current_object->interactive->trans = newt;
      return;
    }
  }
  pop_stack();
  push_number(0);
}
#endif

#ifdef F_TO_UTF8
void f_to_utf8(){
  struct translation *newt = get_translator((char *)sp->u.string);
  pop_stack();
  if(!newt)
    error("unknown encoding");
  char *text = (char *)sp->u.string;
  char *translated = translate_easy(newt->incoming, text);
  pop_stack();
  copy_and_push_string(translated);
}
#endif

#ifdef F_UTF8_TO
void f_utf8_to(){
  struct translation *newt = get_translator((char *)sp->u.string);
  pop_stack();
  if(!newt)
    error("unknown encoding");
  char *text = (char *)sp->u.string;
  char *translated = translate_easy(newt->outgoing, text);
  pop_stack();
  copy_and_push_string(translated);
}
#endif

#ifdef F_STR_TO_ARR
void f_str_to_arr(){
  static struct translation *newt = 0;
  if(!newt){
    newt = get_translator("UTF-32");
    translate_easy(newt->outgoing, " ");
  }
  int len;
  int *trans = (int *)translate(newt->outgoing, sp->u.string, SVALUE_STRLEN(sp)+1, &len);
  len/=4;
  array_t *arr = allocate_array(len);
  while(len--)
    arr->item[len].u.number = trans[len];
  free_svalue(sp, "str_to_arr");
  put_array(arr);
}

#endif
  
#ifdef F_ARR_TO_STR
void f_arr_to_str(){
  static struct translation *newt = 0;
  if(!newt){
    newt = get_translator("UTF-32");
  }
  int len = sp->u.arr->size;
  int *in = MALLOC(sizeof(int)*(len+1));
  char *trans;
  in[len] = 0;
  while(len--)
    in[len] = sp->u.arr->item[len].u.number;

  trans = translate(newt->incoming, (char *)in, (sp->u.arr->size+1)*4, &len);
  FREE(in);
  pop_stack();
  copy_and_push_string(trans);
}

#endif

#ifdef F_STRWIDTH
void f_strwidth(){
  int len = SVALUE_STRLEN(sp);
  int width = 0;
  int i;
  for(i=0; i<len; i++)
    width += !(((sp->u.string[i]) & 0xc0) == 0x80);
  pop_stack();
  push_number(width);
}

#endif
