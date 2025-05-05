enable_testing()
include(CTest)

add_executable(GeneralTest          GeneralTest.cpp)
add_executable(ProductivityFindTest ProductivityFindTest.cpp)

target_link_libraries(GeneralTest
        PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
        Threads::Threads
)
target_link_libraries(ProductivityFindTest
        PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
        Threads::Threads
)

# Определяем, куда у нас собрался бинарь FuseVFSMain
set(FUSE_MAIN_BIN "${CMAKE_BINARY_DIR}/FuseVFSMain")

# Передаём его в код через макрос, если нужно
target_compile_definitions(GeneralTest
        PRIVATE FUSE_MAIN_BIN_PATH="${FUSE_MAIN_BIN}"
)
target_compile_definitions(ProductivityFindTest
        PRIVATE FUSE_MAIN_BIN_PATH="${FUSE_MAIN_BIN}"
)

include(GoogleTest)

# Регистрируем GeneralTest как *один* тестовый запуск
add_test(NAME FileSystemSuite
        COMMAND $<TARGET_FILE:GeneralTest> --gtest_filter=*)
set_tests_properties(FileSystemSuite PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
        ENVIRONMENT "FUSEFS_BINARY=$<TARGET_FILE:FuseVFSMain>"
)

# Оставляем discover только для другого набора тестов
gtest_discover_tests(ProductivityFindTest
        PROPERTIES
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/Tests
        ENVIRONMENT "FUSEFS_BINARY=$<TARGET_FILE:FuseVFSMain>"
)
