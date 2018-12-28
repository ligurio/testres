CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(testres)
SET(SOURCES testres.c
parse_common.c
parse_junit.c
parse_subunit_v1.c
parse_subunit_v2.c
parse_testanything.c
tf-idf.c
ui_common.c
ui_console.c
ui_http.c
sha1.c)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wfloat-equal")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wundef -Wpointer-arith -Wcast-align -Wshadow")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wswitch-enum -Wunreachable-code -Winit-self")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -Werror -pedantic -std=c++11 ")

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O3")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O0 -g")

SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/../)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

INCLUDE(FindEXPAT)
FIND_PACKAGE(EXPAT REQUIRED)

#FIND_PACKAGE(BISON REQUIRED)
#FIND_PACKAGE(FLEX REQUIRED)

#BISON_TARGET(TAP_Parser parse_testanything.y ${CMAKE_CURRENT_BINARY_DIR}/parse_testanything.c)
#FLEX_TARGET(TAP_Scanner parse_testanything.l ${CMAKE_CURRENT_BINARY_DIR}/parse_testanything.c)
#ADD_FLEX_BISON_DEPENDENCY(TAP_Scanner TAP_Parser)

#BISON_TARGET(SubUnit_v1_Parser parse_subunit_v1.y ${CMAKE_CURRENT_BINARY_DIR}/parse_subunit_v1.c)
#FLEX_TARGET(SubUnit_v1_Scanner parse_subunit_v1.l ${CMAKE_CURRENT_BINARY_DIR}/parse_subunit_v1.c)
#ADD_FLEX_BISON_DEPENDENCY(SubUnit_v1_Scanner SubUnit_v1_Parser)

INCLUDE_DIRECTORIES(${EXPAT_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

ADD_EXECUTABLE(${PROJECT_NAME}
	${SOURCES}
#	${BISON_TAP_Parser_OUTPUTS}
#	${FLEX_TAP_Scanner_OUTPUTS}
#	${BISON_SubUnit_v1_Parser_OUTPUTS}
#	${FLEX_SubUnit_v1_Scanner_OUTPUTS}
)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EXPAT_LIBRARIES} m)

FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
INSTALL(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

#ADD_SUBDIRECTORY(tests)
#ENABLE_TESTING()
#ADD_TEST(NAME Unit COMMAND Test)
