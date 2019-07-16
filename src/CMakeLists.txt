cmake_minimum_required(VERSION 3.10)

set(BIN_SOURCES
testres.c
metrics.c
ui_common.c
ui_console.c
ui_http.c
)

set(LIB_SOURCES
parse_common.h
parse_common.c
parse_subunit_v1.h
parse_subunit_v1.c
parse_subunit_v2.h
parse_subunit_v2.c
parse_junit.h
parse_junit.c
parse_subunit_v2.c
parse_testanything.h
parse_testanything.c
sha1.h
sha1.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wfloat-equal")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wpointer-arith -Wcast-align -Wshadow")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-enum -Wunreachable-code -Winit-self")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Werror -pedantic")
if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0 -g")

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${EXPAT_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

add_library(${PROJECT_NAME}-lib ${LIB_SOURCES})
add_executable(${PROJECT_NAME} ${BIN_SOURCES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}-lib ${EXPAT_LIBRARIES} m)
