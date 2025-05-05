# CMake generated Testfile for 
# Source directory: /home/artem4ik/Projects/FuseVFS/Tests
# Build directory: /home/artem4ik/Projects/FuseVFS/cmake-build-debug/Tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/home/artem4ik/Projects/FuseVFS/cmake-build-debug/Tests/ProductivityFindTest[1]_include.cmake")
add_test([=[FileSystemSuite]=] "/home/artem4ik/Projects/FuseVFS/cmake-build-debug/Tests/GeneralTest" "--gtest_filter=*")
set_tests_properties([=[FileSystemSuite]=] PROPERTIES  ENVIRONMENT "FUSEFS_BINARY=/home/artem4ik/Projects/FuseVFS/cmake-build-debug/FuseVFSMain" WORKING_DIRECTORY "/home/artem4ik/Projects/FuseVFS/cmake-build-debug/Tests" _BACKTRACE_TRIPLES "/home/artem4ik/Projects/FuseVFS/Tests/CMakeLists.txt;34;add_test;/home/artem4ik/Projects/FuseVFS/Tests/CMakeLists.txt;0;")
