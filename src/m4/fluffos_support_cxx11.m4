AC_DEFUN([F_SUPPORT_CXX11], [AC_COMPILE_IFELSE(
  [AC_LANG_SOURCE(
    [[
      #include <vector>
      int main() {
        std::vector<int> v = {0, 1, 2};
        for (auto& i : v);
      }
    ]]
  )], [$1], [$2], [])]
)
