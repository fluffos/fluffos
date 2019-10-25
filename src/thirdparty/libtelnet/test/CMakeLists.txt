enable_testing()

foreach (test_name environ01 environ02 environ03 mssp01 rfc1143 simple01 simple02 ttype01 zmp01 zmp02 zmp03)
    add_test(
        NAME ${test_name}
        COMMAND telnet-test ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.input ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.txt)
endforeach ()