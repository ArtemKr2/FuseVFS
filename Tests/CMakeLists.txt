enable_testing()
include(CTest)

add_executable(TestMainFunctions TestMainFunctions.cpp)
add_executable(ProductivityTest ProductivityTest.cpp)

target_link_libraries(TestMainFunctions
        PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
        Threads::Threads
)
target_link_libraries(ProductivityTest
        PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
        Threads::Threads
)

set(FUSE_MAIN_BIN "${CMAKE_BINARY_DIR}/FuseVFSMain")

target_compile_definitions(TestMainFunctions
        PRIVATE FUSE_MAIN_BIN_PATH="${FUSE_MAIN_BIN}"
)
target_compile_definitions(ProductivityTest
        PRIVATE FUSE_MAIN_BIN_PATH="${FUSE_MAIN_BIN}"
)

include(GoogleTest)

add_test(NAME FileSystemSuite
        COMMAND $<TARGET_FILE:TestMainFunctions> --gtest_filter=*)
set_tests_properties(FileSystemSuite PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
        ENVIRONMENT "FUSEFS_BINARY=$<TARGET_FILE:FuseVFSMain>"
)

gtest_discover_tests(ProductivityTest
        PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
        ENVIRONMENT "FUSEFS_BINARY=$<TARGET_FILE:FuseVFSMain>"
)
