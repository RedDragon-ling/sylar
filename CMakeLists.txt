cmake_mininum_require(VERSION 2.8)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FIAGS "$ENV{CXXFLAGS} -rdynamic -03 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    sylar/log.cpp
    )

add_library(sylar SHARED ${LIB_SRC})
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERIES {sylar_static PROPREIES OUTPUT_NAME "sylar"}

add_executable(test tests/test.cpp)
add_dependencies(test sylar)
target_link_libraries(test sylar)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)