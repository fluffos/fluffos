/**
 * crypto.c
 * 
 * Utilises the OpenSSL crypto library to provide various message digest hashes
 * via a hash() efun.  It works in almost the same manner as the hash function
 * from php, and provides md2, md4, md5, mdc2, sha1 and ripemd160 hashes.  You
 * must link against the ssl link library (add -lssl to system_libs).
 *
 * -- coded by Ajandurah@Demonslair (Mark Lyndoe) 10/03/09
 */

#include <openssl/md2.h>
#include <openssl/md4.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <openssl/mdc2.h>
#include <openssl/ripemd.h>

#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#include "../efun_protos.h"
#endif

#ifdef F_HASH
char *hexdump(unsigned char *data, size_t len)
{
   char *buf;
   int i;
   
   buf = malloc((len*2)+1);
   memset(buf, '\0', (len*2));
   
   for (i=0; i<len; i++)
   {
      sprintf(strchr(buf,0), "%02x", data[i]);
   }
   
   return buf;
}

void f_hash(void)
{
   const char *algo;
   const char *data;
   char *res;
   
   algo = (sp - 1)->u.string;
   data = sp->u.string;

   /* MD2 Digest */
   if (strcasecmp(algo, (const char *)"md2") == 0)
   {
      unsigned char md[MD2_DIGEST_LENGTH];
      MD2((unsigned char *)data, strlen(data), md);
      res = hexdump(md, MD2_DIGEST_LENGTH);
   }
   
   /* MD4 Digest */
   else if (strcasecmp(algo, (const char *)"md4") == 0)
   {
      unsigned char md[MD4_DIGEST_LENGTH];
      MD4((unsigned char *)data, strlen(data), md);      
      res = hexdump(md, MD4_DIGEST_LENGTH);      
   }

   /* MD5 Digest */
   else if (strcasecmp(algo, (const char *)"md5") == 0)
   {
      unsigned char md[MD5_DIGEST_LENGTH];
      MD5((unsigned char *)data, strlen(data), md);      
      res = hexdump(md, MD5_DIGEST_LENGTH);      
   }
   
   /* MDC2 Digest */
   else if (strcasecmp(algo, (const char *)"mdc2") == 0)
   {
      unsigned char md[MDC2_DIGEST_LENGTH];
      MDC2((unsigned char *)data, strlen(data), md);      
      res = hexdump(md, MDC2_DIGEST_LENGTH);      
   }
   
   /* RIPEMD160 Digest */
   else if (strcasecmp(algo, (const char *)"ripemd160") == 0)
   {
      unsigned char md[RIPEMD160_DIGEST_LENGTH];
      RIPEMD160((unsigned char *)data, strlen(data), md);      
      res = hexdump(md, RIPEMD160_DIGEST_LENGTH);      
   }
   
   /* SHA1 Digest */
   else if (strcasecmp(algo, (const char *)"sha1") == 0)
   {
      unsigned char md[SHA_DIGEST_LENGTH];
      SHA1((unsigned char *)data, strlen(data), md);
      res = hexdump(md, SHA_DIGEST_LENGTH);
   }
   
   else
   {
      pop_stack();
      res = malloc(29 + strlen(algo));
      sprintf(res, "hash() unknown hash type: %s.\n", algo);
      error(res);
   }
   
   /* Pop the arguments off the stack and push the result */
   free_string_svalue(sp--);
   free_string_svalue(sp);

   sp->subtype = STRING_MALLOC;
   sp->u.string = res;
}
#endif
