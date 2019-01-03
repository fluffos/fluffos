/*
 * build_applies.cc , Original from edit_source.cc
 *
 * This tool parse "applies" list and generate applies_table.cc/applies_table.h.
 */

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cctype>

#include <iostream>
#include <string>

static const char *APPLIES = "vm/internal/applies";
static const char *APPLIES_H = "applies_table.autogen.h";
static const char *APPLIES_TABLE = "applies_table.cc";

int main(int argc, char **argv) {
  if(argc != 2)
  {
      std::cerr << "error:\n" << argv[0] << " <src_dir>\n\n";
      return 1;
  }
  std::string applies_path {argv[1]};
  applies_path += '/';
  applies_path += APPLIES;
  FILE *f = fopen(applies_path.c_str(), "r");
  FILE *out = fopen(APPLIES_H, "w");
  FILE *table = fopen(APPLIES_TABLE, "w");
  char buf[8192];
  char *colon;
  char *p;
  int apply_number = 0;

  fprintf(out,
          "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
          "// !!! This file is automatically generated by build_applies. !!!\n"
          "// !!! do not make any manual changes to this file.           !!!\n"
          "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
          "#ifndef __APPLIES_HH__\n#define __APPLIES_HH__\n\n"
          "extern const char *applies_table[];\n\n// the folowing "
          "must be the first character of __INIT\n#define "
          "APPLY___INIT_SPECIAL_CHAR\t\t'#'\n");
  fprintf(table,
          "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
          "// !!! This file is automatically generated by build_applies. !!!\n"
          "// !!! do not make any manual changes to this file.           !!!\n"
          "// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n\n"
          "const char *applies_table[] = "
          "{\n");

  while (fgets(buf, 8192, f)) {
    buf[strlen(buf) - 1] = 0;
    if (buf[0] == '#') {
      break;
    }
    if ((colon = strchr(buf, ':'))) {
      *colon++ = 0;
      fprintf(out, "#define APPLY_%-30s\t\"%s\"\n", buf, colon);
    } else {
      fprintf(out, "#define APPLY_%-30s\t", buf);
      p = buf;
      while (*p) {
        *p = tolower(*p);
        p++;
      }
      fprintf(out, "\"%s\"\n", buf);
    }
  }
  while (fgets(buf, 8192, f)) {
    buf[strlen(buf) - 1] = 0;
    if ((colon = strchr(buf, ':'))) {
      *colon++ = 0;
      fprintf(table, "\t\"%s\",\n", colon);
      fprintf(out, "#define APPLY_%-30s\t%i\n", buf, apply_number++);
    } else {
      fprintf(out, "#define APPLY_%-30s\t%i\n", buf, apply_number++);
      p = buf;
      while (*p) {
        *p = tolower(*p);
        p++;
      }
      fprintf(table, "\t\"%s\",\n", buf);
    }
  }

  fprintf(table, "};\n");
  fprintf(out, "\n#define NUM_MASTER_APPLIES\t%i\n\n#endif\n", apply_number);

  fclose(out);
  fclose(table);
  fclose(f);

  return 0;
}
