project(libtestoutput)

cmake_minimum_required(VERSION 3.9.2)

function(GENERATE_PARSER)
    cmake_parse_arguments(PARSER "" "FORMAT" "" ${ARGN})
	message("Generate parser of ${PARSER_FORMAT}")

	find_program(YACC_EXECUTABLE yacc)
	if(YACC_EXECUTABLE STREQUAL "YACC_EXECUTABLE-NOTFOUND")
		message(FATAL_ERROR "yacc is not found")
	endif(YACC_EXECUTABLE STREQUAL "YACC_EXECUTABLE-NOTFOUND")

	add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/parse_${PARSER_FORMAT}.tab.c
			${CMAKE_CURRENT_BINARY_DIR}/parse_${PARSER_FORMAT}.tab.h
	COMMAND ${YACC_EXECUTABLE}
	ARGS -b "parse_${PARSER_FORMAT}"
		-d ${CMAKE_CURRENT_SOURCE_DIR}/${PARSER_FORMAT}.y
		DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PARSER_FORMAT}.c)

	set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/parse_${PARSER_FORMAT}.tab.c GENERATED)
	set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/parse_${PARSER_FORMAT}.tab.h GENERATED)
endfunction()

function(GENERATE_LEXER)
    cmake_parse_arguments(LEXER "" "FORMAT" "" ${ARGN})
	message("Generate lexer of ${LEXER_FORMAT}")

	find_program(LEX_EXECUTABLE flex)
	if(LEX_EXECUTABLE STREQUAL "LEX_EXECUTABLE-NOTFOUND")
		message(FATAL_ERROR "lex is not found")
	endif(LEX_EXECUTABLE STREQUAL "LEX_EXECUTABLE-NOTFOUND")

	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${LEXER_FORMAT}.c
	  COMMAND ${LEX_EXECUTABLE}
	  ARGS -o ${CMAKE_CURRENT_BINARY_DIR}/${LEXER_FORMAT}.c
		   ${CMAKE_CURRENT_SOURCE_DIR}/${LEXER_FORMAT}.l)
	set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/${LEXER_FORMAT}.c GENERATED)
endfunction()

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
testanything.c
parse_testanything.tab.c
parse_testanything.tab.h
sha1.h
sha1.c
)

generate_lexer(FORMAT "testanything")
generate_parser(FORMAT "testanything")

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${EXPAT_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
add_library(testoutput ${SOURCE_FILES})

if(BUILD_TESTING)
	add_subdirectory(tests)
endif()
