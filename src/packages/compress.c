/* Compression efun stuff.
 * Started Wed Mar 21 01:52:25 PST 2001
 * by David Bennett (ddt@discworld.imaginary.com)
 */

#ifdef LATTICE
#include "/lpc_incl.h"
#include "/file_incl.h"
#include "/file.h"
#else
#include "../lpc_incl.h"
#include "../file_incl.h"
#include "../file.h"
#endif

#include <zlib.h>

#define GZ_EXTENSION ".gz"

#define COMPRESS_BUF_SIZE 8096

#ifdef F_COMPRESS_FILE
void f_compress_file PROT((void))
{
   int read;
   int len;
   int num_arg = st_num_arg;
   char* input_file;
   char* output_file;
   char* real_input_file;
   char* real_output_file;
   gzFile out_file;
   FILE* in_file;
   char buf[4096];

   // Not a string?  Error!
   if ((sp - num_arg + 1)->type != T_STRING) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   input_file = (sp - num_arg + 1)->u.string;
   if (num_arg == 2) {
      if (((sp - num_arg + 2)->type != T_STRING)) {
         pop_n_elems(num_arg);
         push_number(0);
         return ;
      }
      output_file = (sp - num_arg + 2)->u.string;
   } else {
      len = strlen(input_file);
      if (!strcmp(input_file + len - strlen(GZ_EXTENSION), GZ_EXTENSION)) {
         // Already compressed...
         pop_n_elems(num_arg);
         push_number(0);
         return ;
      }
      output_file = new_string(strlen(input_file) + strlen(GZ_EXTENSION),
                            "compress_file");
      strcpy(output_file, input_file);
      strcat(output_file, GZ_EXTENSION);
   }

   real_output_file = check_valid_path(output_file, current_object, "compress_file", 0);
   if (!real_output_file) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }
   // Copy it into our little buffer.
   strcpy(output_file, real_output_file);

   real_input_file = check_valid_path(input_file, current_object, "compress_file", 0);
   if (!real_input_file) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   in_file = fopen(real_input_file, "r");
   if (!in_file) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   out_file = gzopen(output_file, "w");
   if (!out_file) {
      fclose(in_file);
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   do {
      read = fread(buf, 1, 4096, in_file);
      gzwrite(out_file, buf, read);
   } while (read == 4096);
   fclose(in_file);
   gzclose(out_file);

   unlink(real_input_file);

   if (num_arg != 2) {
      FREE_MSTR(output_file);
   }

   pop_n_elems(num_arg);
   push_number(1);
}
#endif

#ifdef F_UNCOMPRESS_FILE
void f_uncompress_file PROT((void))
{
   int read;
   int len;
   int num_arg = st_num_arg;
   char* input_file;
   char* output_file;
   char* real_input_file;
   char* real_output_file;
   FILE* out_file;
   gzFile in_file;
   char buf[4096];

   // Not a string?  Error!
   if ((sp - num_arg + 1)->type != T_STRING) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   input_file = (sp - num_arg + 1)->u.string;
   if (num_arg == 2) {
      if (((sp - num_arg + 2)->type != T_STRING)) {
         pop_n_elems(num_arg);
         push_number(0);
         return ;
      }
      output_file = (sp - num_arg + 2)->u.string;
   } else {
      len = strlen(input_file);
      if (strcmp(input_file + len - strlen(GZ_EXTENSION), GZ_EXTENSION)) {
         // Not compressed...
         pop_n_elems(num_arg);
         push_number(0);
         return ;
      }
      output_file = new_string(strlen(input_file),
                            "compress_file");
      strcpy(output_file, input_file);
      output_file[len - strlen(GZ_EXTENSION)] = 0;
   }

   real_output_file = check_valid_path(output_file, current_object, "compress_file", 0);
   if (!real_output_file) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }
   // Copy it into our little buffer.
   strcpy(output_file, real_output_file);

   real_input_file = check_valid_path(input_file, current_object, "compress_file", 0);
   if (!real_input_file) {
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   in_file = gzopen(real_input_file, "r");
   if (!in_file) {
      gzclose(in_file);
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   out_file = fopen(output_file, "w");
   if (!out_file) {
      fclose(out_file);
      pop_n_elems(num_arg);
      push_number(0);
      return ;
   }

   do {
      read = gzread(in_file, buf, 4096);
      fwrite(buf, 1, read, out_file);
   } while (read == 4096);
   gzclose(in_file);
   fclose(out_file);

   unlink(real_input_file);

   if (num_arg != 2) {
      FREE_MSTR(output_file);
   }

   pop_n_elems(num_arg);
   push_number(1);
}
#endif

#ifdef F_COMPRESS
void f_compress PROT((void))
{
   unsigned char* buffer;
   unsigned char* input;
   int size;
   buffer_t* real_buffer;
   uLongf new_size;

   if (sp->type == T_STRING) {
      size = SVALUE_STRLEN(sp);
      input = (unsigned char*)sp->u.string;
   } else if (sp->type == T_BUFFER) {
      size = sp->u.buf->size;
      input = sp->u.buf->item;
   } else {
      pop_n_elems(st_num_arg);
      push_undefined();
      return ;
   }

   new_size = size;
   // Make it a little larger as specified in the docs.
   buffer = (unsigned char*)DXALLOC(size * 101 / 100 + 12, TAG_TEMPORARY, "compress");
   compress(buffer, &new_size, input, size);

   // Shrink it down.
   pop_n_elems(st_num_arg);
   real_buffer = allocate_buffer(new_size);
   write_buffer(real_buffer, 0, (char *)buffer, new_size);
   FREE(buffer);
   push_buffer(real_buffer);
}
#endif

#ifdef F_UNCOMPRESS
static void* zlib_alloc(void* opaque, unsigned int items, unsigned int size) {
   return CALLOC(items, size);
}

static void zlib_free(void* opaque, void* address) {
   FREE(address);
}

void f_uncompress PROT((void))
{
   z_stream* compress;
   unsigned char compress_buf[COMPRESS_BUF_SIZE];
   unsigned char* output_data = NULL;
   int len;
   int pos;
   buffer_t* buffer;
   int ret;

   if (sp->type == T_BUFFER) {
      buffer = sp->u.buf;
   } else {
      pop_n_elems(st_num_arg);
      push_undefined();
      return ;
   }

   compress = (z_stream *)
            DXALLOC(sizeof(z_stream), TAG_INTERACTIVE,
                    "start_compression");
   compress->next_in = buffer->item;
   compress->avail_in = buffer->size;
   compress->next_out = compress_buf;
   compress->avail_out = COMPRESS_BUF_SIZE;
   compress->zalloc = zlib_alloc;
   compress->zfree = zlib_free;
   compress->opaque = NULL;

   if (inflateInit(compress) != Z_OK) {
      FREE(compress);
      pop_n_elems(st_num_arg);
      push_undefined();
      return ;
   }

   len = 0;
   output_data = NULL;
   do {
      ret = inflate(compress, 0);
      if (ret == Z_OK) {
         pos = len;
         len += COMPRESS_BUF_SIZE - compress->avail_out;
         if (!output_data) {
            output_data = (unsigned char*)DXALLOC(len, TAG_TEMPORARY, "uncompress");
         } else {
            output_data = REALLOC(output_data, len);
         }
         memcpy(output_data + pos, compress_buf, len - pos);
         compress->next_out = compress_buf;
         compress->avail_out = COMPRESS_BUF_SIZE;
      }
   } while (ret == Z_OK);

   inflateEnd(compress);

   pop_n_elems(st_num_arg);

   if (ret == Z_STREAM_END) {
      buffer = allocate_buffer(len);
      memcpy(buffer->item, output_data, len);
      FREE(output_data);
      push_buffer(buffer);
   } else {
      push_undefined();
   }
}
#endif
