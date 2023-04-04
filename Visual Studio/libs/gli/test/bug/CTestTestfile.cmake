# CMake generated Testfile for 
# Source directory: D:/source/Nyxis-Engine/libs/gli/test/bug
# Build directory: D:/source/Nyxis-Engine/Visual Studio/libs/gli/test/bug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(test-bug "D:/source/Nyxis-Engine/bin/Debug/test-bug.exe")
  set_tests_properties(test-bug PROPERTIES  _BACKTRACE_TRIPLES "D:/source/Nyxis-Engine/libs/gli/test/CMakeLists.txt;22;add_test;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;1;glmCreateTestGTC;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(test-bug "D:/source/Nyxis-Engine/bin/Release/test-bug.exe")
  set_tests_properties(test-bug PROPERTIES  _BACKTRACE_TRIPLES "D:/source/Nyxis-Engine/libs/gli/test/CMakeLists.txt;22;add_test;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;1;glmCreateTestGTC;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(test-bug "D:/source/Nyxis-Engine/bin/MinSizeRel/test-bug.exe")
  set_tests_properties(test-bug PROPERTIES  _BACKTRACE_TRIPLES "D:/source/Nyxis-Engine/libs/gli/test/CMakeLists.txt;22;add_test;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;1;glmCreateTestGTC;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(test-bug "D:/source/Nyxis-Engine/bin/RelWithDebInfo/test-bug.exe")
  set_tests_properties(test-bug PROPERTIES  _BACKTRACE_TRIPLES "D:/source/Nyxis-Engine/libs/gli/test/CMakeLists.txt;22;add_test;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;1;glmCreateTestGTC;D:/source/Nyxis-Engine/libs/gli/test/bug/CMakeLists.txt;0;")
else()
  add_test(test-bug NOT_AVAILABLE)
endif()
