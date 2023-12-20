#include "base/package_api.h"

#include <gtest/gtest.h>

#include "ofile.h"

#include <string>
#include <fstream>
#include <sstream>

// Test fixture class
class OFileTest : public ::testing::Test {
 public:
  static void SetUpTestSuite() {
    chdir(TESTSUITE_DIR);

    config_init();
    init_strings();
  }
};

// test existing ofile
TEST_F(OFileTest, TestExistingOFileAndBack) {
  std::string o_file = "./test.o";
  std::ifstream ifs(o_file, std::ios::binary);
  ASSERT_TRUE(ifs);

  std::stringstream s;
  s << ifs.rdbuf();
  std::string content = s.str();
  ASSERT_FALSE(content.empty());

  // parse program name
  OFile result(content);

  // mapping order can change during load, so we only compare first 3 lines
  {
    std::stringstream first3lines(content);
    std::stringstream first3lines2(result.to_ofile());
    for (int i = 0; i < 3; i++) {
      std::string content_line;
      std::getline(first3lines, content_line, '\n');

      std::string result_line;
      std::getline(first3lines2, result_line, '\n');

      ASSERT_FALSE(result_line.empty());
      ASSERT_EQ(content_line, result_line);
    }
  }

  // verify content
  ASSERT_EQ("#/single/tests/efuns/save_object.c", result.program_name);
  ASSERT_EQ(4, result.variables.size());
  ASSERT_EQ("y", result.variables[0].first);
  ASSERT_EQ("a", result.variables[1].first);
  ASSERT_EQ("b", result.variables[2].first);
  ASSERT_EQ("c", result.variables[3].first);

  std::cout << result.to_json().dump(2) << std::endl;

  // verify parse back from json
  OFile result2(result.to_json());

  // mapping order can change during load, so we only compare first 3 lines
  {
    std::stringstream first3lines(result.to_ofile());
    std::stringstream first3lines2(result2.to_ofile());
    for (int i = 0; i < 3; i++) {
      std::string content_line;
      std::getline(first3lines, content_line, '\n');

      std::string result_line;
      std::getline(first3lines2, result_line, '\n');

      ASSERT_FALSE(result_line.empty());
      ASSERT_EQ(content_line, result_line);
    }
  }
}

TEST_F(OFileTest, TestToJsonAndBack) {
  OFile ofile;
  ofile.program_name = "test_program";
  svalue_t sv;
  sv.type = T_NUMBER;
  sv.u.number = 123;
  ofile.variables.push_back(std::make_pair("var1", sv));

  auto j = ofile.to_json();
  ASSERT_FALSE(j.dump().empty());

  OFile ofile2(j);
  std::string ofile_str = ofile2.to_ofile();
  ASSERT_FALSE(ofile_str.empty());

  ASSERT_EQ(ofile.program_name, ofile2.program_name);
  ASSERT_EQ(ofile.variables.size(), ofile2.variables.size());
  for(size_t i = 0; i < ofile.variables.size(); i++) {
    ASSERT_EQ(ofile.variables[i].first, ofile2.variables[i].first);
    ASSERT_EQ(ofile.variables[i].second.type, ofile2.variables[i].second.type);
    ASSERT_EQ(ofile.variables[i].second.u.number, ofile2.variables[i].second.u.number);
  }
}