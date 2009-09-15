/**
 * sha1.c: this file provides the sha1 hashing algorithm
 * based in part on code in the public domain by Niyaz PK.
 * -- coded by Ajandurah@Demonslair 21/02/08
 */

#include <math.h>

#ifdef LATTICE
#include "/lpc_incl.h"
#else
#include "../lpc_incl.h"
#include "../efun_protos.h"
#endif

#define rotateleft(x,n)    ((x<<n) | (x>>(32-n)))
#define rotateright(x,n)   ((x>>n) | (x<<(32-n)))

#ifdef F_SHA1
void f_sha1(void)
{
   unsigned long int h0,h1,h2,h3,h4,a,b,c,d,e,f,k,temp;
   int i, j, m;

   h0 = 0x67452301;
   h1 = 0xEFCDAB89;
   h2 = 0x98BADCFE;
   h3 = 0x10325476;
   h4 = 0xC3D2E1F0;

   unsigned char *str;
   str = (unsigned char *)malloc(SVALUE_STRLEN(sp)+100);
   
   strcpy((char *)str, (const char *)sp->u.string);
   
   int current_length = strlen((const char *)str);
   int original_length = current_length;
   str[current_length] = 0x80;
   str[current_length + 1] = '\0';
   
   char ic = str[current_length];
   current_length++;
   
   int ib = current_length % 64;
   if (ib < 56)
      ib = 56 - ib;
   else
      ib = 120 - ib;
      
   for (i=0; i<ib; i++)
   {
      str[current_length] = 0x00;
      current_length++;
   }
   
   str[current_length + 1] = '\0';
   
   for (i=0; i<6; i++)
   {
      str[current_length] = 0x0;
      current_length++;
   }
   
   str[current_length] = (original_length * 8) / 0x100;
   current_length++;
   str[current_length] = (original_length * 8) % 0x100;
   current_length++;
   str[current_length + i] = '\0';
   
   int number_of_chunks = current_length / 64;
   unsigned long int word[80];
   
   for (i=0; i<number_of_chunks; i++)
   {
      for (j=0; j<16; j++)
      {
         word[j] = str[i*64+j*4+0] * 0x1000000 + str[i*64+j*4+1] * 0x10000 + str[i*64+j*4+2] * 0x100 + str[i*64+j*4+3];
      }
      
      for (j=16; j<80; j++)
      {
         word[j] = rotateleft((word[j-3] ^ word[j-8] ^ word[j-14] ^ word[j-16]),1);
      }
      
      a = h0;
      b = h1;
      c = h2;
      d = h3;
      e = h4;
      
      for (m=0; m<80; m++)
      {
         if (m <= 19)
         {
            f = (b & c) | ((~b) & d);
            k = 0x5A827999;
         }
         else if (m <= 39)
         {
            f = b ^ c ^ d;
            k = 0x6ED9EBA1;
         }
         else if (m <= 59)
         {
            f = (b & c) | (b & d) | (c & d);
            k = 0x8F1BBCDC;
         }
         else
         {
            f = b ^ c ^ d;
            k = 0xCA62C1D6;
         }
         
         temp = (rotateleft(a, 5) + f + e + k + word[m]) & 0xFFFFFFFF;
         e = d;
         d = c;
         c = rotateleft(b, 30);
         b = a;
         a = temp;
      }
      
      h0 = h0 + a;
      h1 = h1 + b;
      h2 = h2 + c;
      h3 = h3 + d;
      h4 = h4 + e;
   }
   
   sprintf((char *)str, "%x%x%x%x%x", h0, h1, h2, h3, h4);
   pop_stack();
   push_malloced_string(string_copy((char *)str, "f_sha1"));
   free(str);
}
#endif
