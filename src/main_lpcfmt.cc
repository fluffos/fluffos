#include "base/std.h"

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fcntl.h>  // for O_RDONLY
#include <sstream>

#include "include/function.h"
#include "efuns.autogen.h"
#include "vm/internal/base/number.h"
#include "compiler/internal/grammar_rules.h"
#include "grammar.autogen.h"
#include "compiler/internal/lex.h"
#include "thirdparty/fmt/include/fmt/format.h"

extern char* outp;  // from lex.cc

namespace {

struct Node_Statement {
  std::vector<std::string> tokens;
};

struct Node_Statement_PreprocessorCommand : Node_Statement {};
struct Node_Statement_FunctionDecl : Node_Statement {};
struct Node_Statement_VariableDecl : Node_Statement {};
struct Node_Statement_Expr : Node_Statement {};

struct Node_Program {
  std::vector<Node_Statement> statements;
};

std::ostringstream output;
std::vector<std::string> current_output_line;

std::unique_ptr<Node_Program> current_program;
std::unique_ptr<Node_Statement> current_statement;

}  // namespace

void add_to_current_line(std::string token) { current_output_line.push_back(token); }

void end_current_line() {
  if (!current_output_line.empty()) {
    for (auto& token : current_output_line) {
      output << token << " ";
    }
    current_output_line.clear();

    output << std::endl;
  }
}

void add_newline(std::string token) {
  end_current_line();

  output << token << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Usage: lpcfmt lpc_file" << std::endl;
    return 1;
  }

  auto real_name = argv[1];

  int f = open(real_name, O_RDONLY);
#ifdef _WIN32
  // TODO: change everything to use fopen instead.
  _setmode(f, _O_BINARY);
#endif
  if (f == -1) {
    std::cerr << "Could not read the file '" << real_name << "'" << std::endl;
    return 1;
  }

  init_strings();
  init_identifiers();

  current_file = make_shared_string(real_name);

  start_new_file(f);

  int has_parse_error = false;

  auto c = yylex(true);
  while (c > 0) {
    if (c < L_STRING) {
      if (c == '\n') {
        end_current_line();
      } else {
        add_to_current_line(fmt::format(FMT_STRING("{:c}"), c));
      }
    } else {
      switch (c) {
        case L_STRING: {
          add_to_current_line("\"" + std::string(yylval.string) + "\"");
          break;
        }
        case L_NUMBER: {
          add_to_current_line(fmt::format(FMT_STRING("{:d}"), yylval.number));
          break;
        }
          //      L_REAL = 260,
        case L_BASIC_TYPE: {
          switch (yylval.number) {
            case TYPE_STRING:
              add_to_current_line("string");
              break;
            case TYPE_NUMBER:
              add_to_current_line("number");
              break;
            case TYPE_REAL:
              add_to_current_line("float");
              break;
            case TYPE_MAPPING:
              add_to_current_line("mapping");
              break;
            case TYPE_OBJECT:
              add_to_current_line("object");
              break;
            case TYPE_BUFFER:
              add_to_current_line("buffer");
              break;
            default:
              add_to_current_line(fmt::format(FMT_STRING("<{:d}>"), yylval.number));
              break;
          }
          break;
        }
        case L_TYPE_MODIFIER: {
          switch (yylval.number) {
            case DECL_NOMASK:
              add_to_current_line("nomask");
              break;
            case DECL_NOSAVE:
              add_to_current_line("nosave");
              break;
            case DECL_PRIVATE:
              add_to_current_line("private");
              break;
            case DECL_PROTECTED:
              add_to_current_line("protected");
              break;
            case DECL_PUBLIC:
              add_to_current_line("public");
              break;
            default:
              add_to_current_line(fmt::format(FMT_STRING("<{:d}>"), yylval.number));
              break;
          }
          break;
        }
          //      L_DEFINED_NAME = 263,
        case L_IDENTIFIER: {
          add_to_current_line(std::string(yylval.string));
          break;
        }
          //      L_EFUN = 265,
          //      L_INC = 266,
          //      L_DEC = 267,
        case L_ASSIGN:
          add_to_current_line("=");
          break;
          //      L_LAND = 269,
          //      L_LOR = 270,
          //      L_LSH = 271,
          //      L_RSH = 272,
          //      L_ORDER = 273,
          //      L_NOT = 274,
          //      L_IF = 275,
          //      L_ELSE = 276,
          //      L_SWITCH = 277,
          //      L_CASE = 278,
          //      L_DEFAULT = 279,
          //      L_RANGE = 280,
          //      L_DOT_DOT_DOT = 281,
          //      L_WHILE = 282,
          //      L_DO = 283,
          //      L_FOR = 284,
          //      L_FOREACH = 285,
          //      L_IN = 286,
          //      L_BREAK = 287,
          //      L_CONTINUE = 288,
          //      L_RETURN = 289,
          //      L_ARROW = 290,
        case L_INHERIT: {
          add_to_current_line("inherit");
          break;
        }
          //      L_COLON_COLON = 292,
          //      L_ARRAY_OPEN = 293,
          //      L_MAPPING_OPEN = 294,
          //      L_FUNCTION_OPEN = 295,
          //      L_NEW_FUNCTION_OPEN = 296,
          //      L_SSCANF = 297,
          //      L_CATCH = 298,
          //      L_TREE = 299,
          //      L_REF = 300,
          //      L_PARSE_COMMAND = 301,
          //      L_TIME_EXPRESSION = 302,
          //      L_CLASS = 303,
          //      L_NEW = 304,
          //      L_PARAMETER = 305,
        case L_PREPROCESSOR_COMMAND: {
          add_newline("#" + std::string(yylval.string));
          break;
        }
          //      LOWER_THAN_ELSE = 307,
          //      L_EQ = 308,
          //      L_NE = 309
        default:
          add_to_current_line(fmt::format(FMT_STRING("<{:d}>"), c));
          std::cerr << "Unknown token: " << c << std::endl;
          // has_parse_error = true;
      }
    }
    c = yylex(true);
  }

  end_new_file();

  if (!has_parse_error) {
    std::cout << output.str() << std::endl;
  }
  return 0;
}
