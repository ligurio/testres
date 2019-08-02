cmake_minimum_required(VERSION 3.9.2)

set(SOURCE_FILES
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
crc32.h
crc32.c
)

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${EXPAT_INCLUDE_DIRS})
add_library(testoutput ${SOURCE_FILES})

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
