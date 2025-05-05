cmake_minimum_required(VERSION 3.26)
project(FuseVFS LANGUAGES CXX)
find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall")


include(FetchContent)

# CLI11
FetchContent_Declare(
        CLI11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG        v2.4.2
)
FetchContent_MakeAvailable(CLI11)

# magic_enum
FetchContent_Declare(
        magic_enum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_TAG        v0.9.1
)
FetchContent_MakeAvailable(magic_enum)

# GoogleTest
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)

FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE FUSEVFS_SOURCES
        "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

add_library(${PROJECT_NAME} STATIC
        ${FUSEVFS_SOURCES}
)

target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        CLI11::CLI11
        magic_enum::magic_enum
        Threads::Threads
)


find_package(PkgConfig REQUIRED)
pkg_search_module(FUSE3 REQUIRED fuse3)

target_include_directories(${PROJECT_NAME} PUBLIC ${FUSE3_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${FUSE3_LIBRARIES})


add_executable(FuseVFSMain main.cpp)
target_link_libraries(FuseVFSMain PRIVATE ${PROJECT_NAME})

add_executable(FuseVFSFind FuseVFSFind.cpp)
target_link_libraries(FuseVFSFind PRIVATE ${PROJECT_NAME})

enable_testing()
add_subdirectory(Tests)