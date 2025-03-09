# CMake generated Testfile for 
# Source directory: C:/Users/barte/Desktop/VoltaRewritten/include
# Build directory: C:/Users/barte/Desktop/VoltaRewritten/include/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(lua-testsuite "C:/Users/barte/Desktop/VoltaRewritten/include/build/lua.exe" "-e" "_U=true" "all.lua")
set_tests_properties(lua-testsuite PROPERTIES  WORKING_DIRECTORY "C:/Users/barte/Desktop/VoltaRewritten/include/lua-5.4.7-tests" _BACKTRACE_TRIPLES "C:/Users/barte/Desktop/VoltaRewritten/include/CMakeLists.txt;31;add_test;C:/Users/barte/Desktop/VoltaRewritten/include/CMakeLists.txt;0;")
subdirs("lua-5.4.7")
