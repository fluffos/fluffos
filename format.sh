find src -iname *.cc -o -iname *.h | grep -v thirdparty | xargs clang-format-9 -i
