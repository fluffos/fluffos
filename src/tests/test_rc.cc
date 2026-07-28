#include <gtest/gtest.h>
#include "base/std.h"

#include "base/internal/external_port.h"
#include "base/internal/rc.h"
#include "base/internal/stralloc.h"

#include <cstdio>
#include <fstream>
#include <functional>
#include <string>

#ifdef _WIN32
#include <io.h>
#define dup _dup
#define dup2 _dup2
#else
#include <unistd.h>
#endif

// read_config() drives every runtime option of the driver, but nothing outside
// a full driver boot exercised it: the min/max clamping, comment stripping,
// port/TLS parsing and the `--generate-config` template were all uncovered.

namespace {

std::string temp_path(const std::string& name) {
  return std::string(testing::TempDir()) + "rc_test_" + name;
}

// read_config()'s fatal paths all call exit(-1). POSIX's wait() status only
// carries the low 8 bits of the exit code (WEXITSTATUS), so -1 truncates to
// 255 there; Windows' ExitProcess() takes the value as a full 32-bit code
// with no truncation, so GetExitCodeProcess() (and gtest's death-test
// matcher, which reads it back as a plain int) sees -1 unchanged.
#ifdef _WIN32
constexpr int kFatalExitCode = -1;
#else
constexpr int kFatalExitCode = 255;
#endif

// Minimal set of options read_config() refuses to run without.
const char* const kRequiredLines =
    "name : testmud\n"
    "mudlib directory : /tmp/mudlib\n"
    "log directory : log\n"
    "include directories : /include\n"
    "master file : /master\n";

std::string write_config(const std::string& name, const std::string& extra,
                         bool with_required = true) {
  auto path = temp_path(name);
  std::ofstream out(path, std::ios::trunc);
  if (with_required) {
    out << kRequiredLines;
  }
  out << extra;
  out.close();
  return path;
}

// Redirect stdout (printf/debug_message) into a file for the duration of fn.
std::string capture_stdout(const std::string& name, const std::function<void()>& fn) {
  auto path = temp_path(name);

  fflush(stdout);
  int const saved = dup(1);
  FILE* redirected = freopen(path.c_str(), "w", stdout);
  EXPECT_NE(nullptr, redirected);

  fn();

  fflush(stdout);
  dup2(saved, 1);
  close(saved);
  clearerr(stdout);

  std::ifstream in(path);
  return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

class RcTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() { init_strings(); }

  // read_config() chatters to stdout; keep the test output readable.
  static void quiet_read_config(const std::string& path) {
    capture_stdout("read_config.log", [&]() { read_config(path.c_str()); });
  }
};

}  // namespace

TEST_F(RcTest, RequiredStringsAndDefaultsAreParsed) {
  auto path = write_config("basic.cfg", "");
  quiet_read_config(path);

  EXPECT_STREQ("testmud", CONFIG_STR(__MUD_NAME__));
  EXPECT_STREQ("/tmp/mudlib", CONFIG_STR(__MUD_LIB_DIR__));
  EXPECT_STREQ("log", CONFIG_STR(__LOG_DIR__));
  EXPECT_STREQ("/include", CONFIG_STR(__INCLUDE_DIRS__));
  EXPECT_STREQ("/master", CONFIG_STR(__MASTER_FILE__));

  // omitted options keep their compiled-in defaults
  EXPECT_EQ(600, CONFIG_INT(__TIME_TO_CLEAN_UP__));
  EXPECT_EQ(900, CONFIG_INT(__TIME_TO_RESET__));
  EXPECT_EQ(64, CONFIG_INT(__MAX_LOCAL_VARIABLES__));

  // omitted strings are still allocated, never left NULL
  EXPECT_STREQ("", CONFIG_STR(__MUD_IP__));
  EXPECT_STREQ("", CONFIG_STR(__DEFAULT_ERROR_MESSAGE__));
}

TEST_F(RcTest, CommentsBlankLinesAndSurroundingSpaceAreIgnored) {
  auto path = write_config("comments.cfg",
                           "# a full line comment\n"
                           "\n"
                           "   \t \n"
                           "  time to reset : 1234   # trailing comment\n"
                           "mud ip : 127.0.0.1\n");
  quiet_read_config(path);

  EXPECT_EQ(1234, CONFIG_INT(__TIME_TO_RESET__));
  EXPECT_STREQ("127.0.0.1", CONFIG_STR(__MUD_IP__));
}

TEST_F(RcTest, OutOfRangeIntegersFallBackToDefault) {
  auto path = write_config("range.cfg",
                           "maximum local variables : 10\n"  // min is 64
                           "time to reset : -5\n");          // min is 0
  quiet_read_config(path);

  EXPECT_EQ(64, CONFIG_INT(__MAX_LOCAL_VARIABLES__));
  EXPECT_EQ(900, CONFIG_INT(__TIME_TO_RESET__));

  // ...while an in-range value is accepted
  path = write_config("range_ok.cfg", "maximum local variables : 200\n");
  quiet_read_config(path);
  EXPECT_EQ(200, CONFIG_INT(__MAX_LOCAL_VARIABLES__));
}

TEST_F(RcTest, UnquotedGlobalIncludeFileGetsQuoted) {
  auto path = write_config("gif.cfg", "global include file : /include/globals.h\n");
  quiet_read_config(path);
  EXPECT_STREQ("\"/include/globals.h\"", CONFIG_STR(__GLOBAL_INCLUDE_FILE__));

  path = write_config("gif_quoted.cfg", "global include file : \"/include/globals.h\"\n");
  quiet_read_config(path);
  EXPECT_STREQ("\"/include/globals.h\"", CONFIG_STR(__GLOBAL_INCLUDE_FILE__));

  path = write_config("gif_angle.cfg", "global include file : <globals.h>\n");
  quiet_read_config(path);
  EXPECT_STREQ("<globals.h>", CONFIG_STR(__GLOBAL_INCLUDE_FILE__));
}

TEST_F(RcTest, DefaultFailMessageGetsNewlineAndFallback) {
  auto path = write_config("dfm_missing.cfg", "");
  quiet_read_config(path);
  // the built-in fallback already ends in a newline and still gets one appended
  EXPECT_STREQ("What?\n\n", CONFIG_STR(__DEFAULT_FAIL_MESSAGE__));

  path = write_config("dfm.cfg", "default fail message : Huh?\n");
  quiet_read_config(path);
  EXPECT_STREQ("Huh?\n", CONFIG_STR(__DEFAULT_FAIL_MESSAGE__));
}

TEST_F(RcTest, ExternalPortsAreParsedByKind) {
  auto path = write_config("ports.cfg",
                           "external_port_1 : telnet 5000\n"
                           "external_port_2 : binary 5001\n"
                           "external_port_3 : ascii 5002\n"
                           "external_port_4 : MUD 5003\n");
  quiet_read_config(path);

  EXPECT_EQ(PORT_TYPE_TELNET, external_port[0].kind);
  EXPECT_EQ(5000, external_port[0].port);
  EXPECT_EQ(PORT_TYPE_BINARY, external_port[1].kind);
  EXPECT_EQ(5001, external_port[1].port);
  EXPECT_EQ(PORT_TYPE_ASCII, external_port[2].kind);
  EXPECT_EQ(5002, external_port[2].port);
  EXPECT_EQ(PORT_TYPE_MUD, external_port[3].kind);
  EXPECT_EQ(5003, external_port[3].port);
  EXPECT_EQ(PORT_TYPE_UNDEFINED, external_port[4].kind);
}

// The legacy "port number" line defines external_port_1 implicitly, and an
// explicit external_port_1 alongside it is ignored (with a warning).
TEST_F(RcTest, LegacyPortNumberDefinesFirstPort) {
  auto path = write_config("legacy_port.cfg",
                           "port number : 4000\n"
                           "external_port_1 : binary 4321\n"
                           "external_port_2 : binary 4001\n");
  auto log = capture_stdout("legacy_port.log", [&]() { read_config(path.c_str()); });

  EXPECT_EQ(4000, CONFIG_INT(__MUD_PORT__));
  EXPECT_EQ(PORT_TYPE_TELNET, external_port[0].kind);
  EXPECT_EQ(4000, external_port[0].port);
  EXPECT_EQ(PORT_TYPE_BINARY, external_port[1].kind);
  EXPECT_EQ(4001, external_port[1].port);
  EXPECT_NE(std::string::npos, log.find("external_port_1 already defined"));
}

TEST_F(RcTest, PortTlsCertAndKeyAreParsed) {
  auto path = write_config("tls.cfg",
                           "external_port_1 : telnet 5000\n"
                           "external_port_1_tls : cert=etc/cert.pem key=etc/key.pem\n");
  quiet_read_config(path);

  EXPECT_EQ("etc/cert.pem", external_port[0].tls_cert);
  EXPECT_EQ("etc/key.pem", external_port[0].tls_key);
}

TEST_F(RcTest, WebsocketPortReadsHttpDir) {
  auto path = write_config("ws.cfg",
                           "external_port_1 : websocket 8080\n"
                           "websocket http dir : www\n");
  quiet_read_config(path);

  EXPECT_EQ(PORT_TYPE_WEBSOCKET, external_port[0].kind);
  EXPECT_EQ(8080, external_port[0].port);
  EXPECT_STREQ("www", CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__));
}

TEST_F(RcTest, ObsoleteAndMissingRecommendedLinesWarn) {
  auto path = write_config("obsolete.cfg",
                           "swap file : /tmp/swap\n"  // obsolete: warn if found
                           "wombles : 1\n");
  auto log = capture_stdout("obsolete.log", [&]() { read_config(path.c_str()); });

  EXPECT_NE(std::string::npos, log.find("obsolete line in config file"));
  EXPECT_NE(std::string::npos, log.find("swap file"));
  // "debug log file" is recommended, not required: missing means a warning
  EXPECT_NE(std::string::npos, log.find("Missing line in config file"));
  EXPECT_NE(std::string::npos, log.find("debug log file"));
}

TEST_F(RcTest, PrintRcTableShowsValuesAndDefaults) {
  auto path = write_config("print.cfg", "time to reset : 1234\n");
  quiet_read_config(path);

  auto out = capture_stdout("print.log", []() { print_rc_table(); });

  EXPECT_NE(std::string::npos, out.find("time to reset : 1234 # default: 900"));
  EXPECT_NE(std::string::npos, out.find("time to clean up : 600\n"));
}

// `driver --generate-config` must emit a file the parser accepts: this pins
// the template against the tables it is generated from.
TEST_F(RcTest, GeneratedConfigTemplateIsParseable) {
  auto template_path = temp_path("template.cfg");
  {
    fflush(stdout);
    int const saved = dup(1);
    ASSERT_NE(nullptr, freopen(template_path.c_str(), "w", stdout));
    print_config_template();
    fflush(stdout);
    dup2(saved, 1);
    close(saved);
    clearerr(stdout);
  }

  quiet_read_config(template_path);

  EXPECT_STREQ("CHANGE_ME", CONFIG_STR(__MUD_NAME__));
  EXPECT_STREQ("/path/to/your/mudlib", CONFIG_STR(__MUD_LIB_DIR__));
  EXPECT_EQ(PORT_TYPE_TELNET, external_port[0].kind);
  EXPECT_EQ(4000, external_port[0].port);
  // template values are the compiled-in defaults
  EXPECT_EQ(600, CONFIG_INT(__TIME_TO_CLEAN_UP__));
  EXPECT_EQ(900, CONFIG_INT(__TIME_TO_RESET__));
}

TEST_F(RcTest, MissingRequiredOptionIsFatal) {
  auto path = write_config("missing_name.cfg",
                           "mudlib directory : /tmp/mudlib\n"
                           "log directory : log\n"
                           "include directories : /include\n"
                           "master file : /master\n",
                           /* with_required = */ false);

  EXPECT_EXIT(read_config(path.c_str()), ::testing::ExitedWithCode(kFatalExitCode), ".*");
}

TEST_F(RcTest, UnknownPortKindIsFatal) {
  auto path = write_config("bad_port.cfg", "external_port_1 : carrier_pigeon 5000\n");

  EXPECT_EXIT(read_config(path.c_str()), ::testing::ExitedWithCode(kFatalExitCode), ".*");
}

TEST_F(RcTest, MissingConfigFileIsFatal) {
  EXPECT_EXIT(read_config(temp_path("does_not_exist.cfg").c_str()), ::testing::ExitedWithCode(kFatalExitCode),
              ".*");
}
