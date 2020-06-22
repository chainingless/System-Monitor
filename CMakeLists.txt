cmake_minimum_required(VERSION 2.6)
project(monitor)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

include_directories(include)
file(GLOB SOURCES "src/*.cpp")
file(GLOB INCLUDES "include/*.h")

add_executable(monitor ${SOURCES} ${INCLUDES})

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(LINUX)
  set_source_files_properties(src/macos_parser.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
  source_group("osx" src/macos_parser.cpp)
else(APPLE)
  set_source_files_properties(src/linux_parser.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
  source_group("linux" src/linux_parser.cpp)
endif()

set_property(TARGET monitor PROPERTY CXX_STANDARD 17)
target_link_libraries(monitor ${CURSES_LIBRARIES})
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(monitor PRIVATE -Wall -Wextra -DNCURSES_OPAQUE=0)
else()
  target_compile_options(monitor PRIVATE -Wall -Wextra)
endif()
