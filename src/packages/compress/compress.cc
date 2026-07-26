/* Compression efun stuff.
 * Started Wed Mar 21 01:52:25 PST 2001
 * by David Bennett (ddt@discworld.imaginary.com)
 */

#include "base/package_api.h"

#include "packages/core/file.h"

#include <zlib.h>

#define GZ_EXTENSION ".gz"

enum { COMPRESS_BUF_SIZE = 8096 };

#ifdef F_COMPRESS_FILE
void f_compress_file() {
  int readb;
  int len;
  int const num_arg = st_num_arg;
  const char* input_file;
  const char* output_file;
  const char* real_input_file;
  const char* real_output_file;
  char* tmpout;
  gzFile out_file;
  FILE* in_file;
  char buf[4096];
  char outname[1024];

  // Not a string?  Error!
  if ((sp - num_arg + 1)->type != T_STRING) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  input_file = (sp - num_arg + 1)->u.string;
  if (num_arg == 2) {
    if (((sp - num_arg + 2)->type != T_STRING)) {
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }
    output_file = (sp - num_arg + 2)->u.string;
  } else {
    len = strlen(input_file);
    if (!strcmp(input_file + len - strlen(GZ_EXTENSION), GZ_EXTENSION)) {
      // Already compressed...
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }
    tmpout = new_string(strlen(input_file) + strlen(GZ_EXTENSION), "compress_file");
    strcpy(tmpout, input_file);
    strcat(tmpout, GZ_EXTENSION);
    output_file = tmpout;
  }

  real_output_file = check_valid_path(output_file, current_object, "compress_file", 1);
  if (!real_output_file) {
    if (num_arg != 2) {
      FREE_MSTR(output_file);
    }
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }
  if (strlen(real_output_file) >= sizeof(outname)) {
    if (num_arg != 2) {
      FREE_MSTR(output_file);
    }
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }
  // Copy it into our little buffer.
  strcpy(outname, real_output_file);
  // Free the old file.
  if (num_arg != 2) {
    FREE_MSTR(output_file);
  }
  output_file = outname;

  real_input_file = check_valid_path(input_file, current_object, "compress_file", 0);
  if (!real_input_file) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  in_file = fopen(real_input_file, "rb");
  if (!in_file) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  out_file = gzopen(output_file, "wb");
  if (!out_file) {
    fclose(in_file);
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  bool ok = true;
  do {
    readb = fread(buf, 1, 4096, in_file);
    if (readb > 0 && gzwrite(out_file, buf, readb) != readb) {
      ok = false;
      break;
    }
  } while (readb == 4096);
  if (ferror(in_file)) {
    ok = false;
  }
  fclose(in_file);
  // The deflate stream is flushed by gzclose(); a full disk shows up there.
  if (gzclose(out_file) != Z_OK) {
    ok = false;
  }

  // Only consume the source once the compressed copy is known to be complete,
  // otherwise a failed compress destroys the only copy of the data.
  if (!ok) {
    debug_message("compress_file: failed to compress /%s into /%s.\n", real_input_file,
                  output_file);
    unlink(output_file);
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  unlink(real_input_file);

  pop_n_elems(num_arg);
  push_number(1);
}
#endif

#ifdef F_UNCOMPRESS_FILE
void f_uncompress_file() {
  int readb;
  int len;
  int const num_arg = st_num_arg;
  const char* input_file;
  const char* output_file;
  const char* real_input_file;
  const char* real_output_file;
  FILE* out_file;
  gzFile in_file;
  char buf[4196];
  char outname[1024];

  // Not a string?  Error!
  if ((sp - num_arg + 1)->type != T_STRING) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  input_file = (sp - num_arg + 1)->u.string;
  if (num_arg == 2) {
    if (((sp - num_arg + 2)->type != T_STRING)) {
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }
    output_file = (sp - num_arg + 2)->u.string;
  } else {
    char* tmp;
    len = strlen(input_file);
    if (strcmp(input_file + len - strlen(GZ_EXTENSION), GZ_EXTENSION) != 0) {
      // Not compressed...
      pop_n_elems(num_arg);
      push_number(0);
      return;
    }
    tmp = new_string(len, "compress_file");
    strcpy(tmp, input_file);
    tmp[len - strlen(GZ_EXTENSION)] = 0;
    output_file = tmp;
  }

  real_output_file = check_valid_path(output_file, current_object, "compress_file", 1);
  if (!real_output_file) {
    if (num_arg != 2) {
      FREE_MSTR(output_file);
    }

    pop_n_elems(num_arg);
    push_number(0);
    return;
  }
  if (strlen(real_output_file) >= sizeof(outname)) {
    if (num_arg != 2) {
      FREE_MSTR(output_file);
    }
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }
  // Copy it into our little buffer.
  strcpy(outname, real_output_file);
  if (num_arg != 2) {
    FREE_MSTR(output_file);
  }
  output_file = outname;

  real_input_file = check_valid_path(input_file, current_object, "compress_file", 0);
  if (!real_input_file) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  in_file = gzopen(real_input_file, "rb");
  if (!in_file) {
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  out_file = fopen(output_file, "wb");
  if (!out_file) {
    gzclose(in_file);
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  bool ok = true;
  do {
    readb = gzread(in_file, buf, 4096);
    // gzread() returns -1 on a read/inflate error (truncated or corrupt
    // gzip data): passing that straight to fwrite() converts it to a huge
    // size_t count.
    if (readb < 0) {
      ok = false;
      break;
    }
    if (readb > 0 && fwrite(buf, 1, readb, out_file) != static_cast<size_t>(readb)) {
      ok = false;
      break;
    }
  } while (readb == 4096);
  gzclose(in_file);
  if (fclose(out_file) != 0) {
    ok = false;
  }

  // Only consume the source once the decompressed copy is known to be
  // complete, otherwise a failed decompress destroys the only copy.
  if (!ok) {
    debug_message("uncompress_file: failed to decompress /%s into /%s.\n", real_input_file,
                  output_file);
    unlink(output_file);
    pop_n_elems(num_arg);
    push_number(0);
    return;
  }

  unlink(real_input_file);

  pop_n_elems(num_arg);
  push_number(1);
}
#endif

#ifdef F_COMPRESS
void f_compress() {
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
    return;
  }

  new_size = compressBound(size);
  // Make it a little larger as specified in the docs.
  buffer = reinterpret_cast<unsigned char*>(DMALLOC(new_size, TAG_TEMPORARY, "compress"));
  compress(buffer, &new_size, input, size);

  // Shrink it down.
  pop_n_elems(st_num_arg);
  real_buffer = allocate_buffer(new_size);
  write_buffer(real_buffer, 0, reinterpret_cast<char*>(buffer), new_size);
  FREE(buffer);
  push_refed_buffer(real_buffer);
}
#endif

#ifdef F_UNCOMPRESS
static void* zlib_alloc(void* /*opaque*/, unsigned int items, unsigned int size) {
  return DCALLOC(items, size, TAG_TEMPORARY, "zlib_alloc");
}

static void zlib_free(void* /*opaque*/, void* address) { FREE(address); }

void f_uncompress() {
  z_stream* compressed;
  unsigned char compress_buf[COMPRESS_BUF_SIZE];
  unsigned char* output_data = nullptr;
  int len;
  int pos;
  buffer_t* buffer;
  int ret;

  if (sp->type == T_BUFFER) {
    buffer = sp->u.buf;
  } else {
    pop_n_elems(st_num_arg);
    push_undefined();
    return;
  }

  compressed =
      reinterpret_cast<z_stream*>(DMALLOC(sizeof(z_stream), TAG_INTERACTIVE, "start_compression"));
  compressed->next_in = buffer->item;
  compressed->avail_in = buffer->size;
  compressed->next_out = compress_buf;
  compressed->avail_out = COMPRESS_BUF_SIZE;
  compressed->zalloc = zlib_alloc;
  compressed->zfree = zlib_free;
  compressed->opaque = nullptr;

  if (inflateInit(compressed) != Z_OK) {
    FREE(compressed);
    pop_n_elems(st_num_arg);
    error("inflateInit failed");
  }

  len = 0;
  output_data = nullptr;
  bool too_large = false;
  do {
    ret = inflate(compressed, 0);
    if (ret == Z_OK || ret == Z_STREAM_END) {
      pos = len;
      len += COMPRESS_BUF_SIZE - compressed->avail_out;
      if (len > CONFIG_INT(__MAX_BUFFER_SIZE__)) {
        // Decompressed output has grown past the configured limit (e.g. a
        // zip bomb) -- stop here, before `len` can overflow the plain int
        // used for the DMALLOC/DREALLOC/memcpy sizes below.
        too_large = true;
        break;
      }
      if (!output_data) {
        output_data = reinterpret_cast<unsigned char*>(DMALLOC(len, TAG_TEMPORARY, "uncompress"));
      } else {
        output_data = reinterpret_cast<unsigned char*>(
            DREALLOC(output_data, len, TAG_TEMPORARY, "uncompress"));
      }
      memcpy(output_data + pos, compress_buf, len - pos);
      compressed->next_out = compress_buf;
      compressed->avail_out = COMPRESS_BUF_SIZE;
    }
  } while (ret == Z_OK);

  inflateEnd(compressed);

  pop_n_elems(st_num_arg);

  if (too_large) {
    if (output_data) {
      FREE(output_data);
    }
    FREE(compressed);
    error("uncompress: decompressed data exceeds maximum buffer size\n");
  }

  if (ret == Z_STREAM_END) {
    buffer = allocate_buffer(len);
    write_buffer(buffer, 0, reinterpret_cast<char*>(output_data), len);
    FREE(output_data);
    push_refed_buffer(buffer);
    FREE(compressed);
  } else {
    if (output_data) {
      FREE(output_data);
    }
    FREE(compressed);
    error("inflate: no ZSTREAM_END\n");
  }
}
#endif
