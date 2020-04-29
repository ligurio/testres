cmake_minimum_required (VERSION 3.9.2)

set(MODULE_NAME "TestLibTestOutput")
set(MODULE_PREFIX "TEST_LIB_TEST_OUTPUT")

set(${MODULE_PREFIX}_DRIVER ${MODULE_NAME}.c)

set(${MODULE_PREFIX}_TESTS
		TestParseJUnit.c
		TestParseSubunitV1.c
		TestParseSubunitV2.c
		TestParseTestanything.c)

create_test_sourcelist(${MODULE_PREFIX}_SRCS
        ${${MODULE_PREFIX}_DRIVER}
        ${${MODULE_PREFIX}_TESTS})

include_directories(..)
add_executable(${MODULE_NAME} ${${MODULE_PREFIX}_SRCS})
set(${MODULE_PREFIX}_LIBS testoutput ${EXPAT_LIBRARIES} m)
target_link_libraries(${MODULE_NAME} ${${MODULE_PREFIX}_LIBS})

foreach(test ${${MODULE_PREFIX}_TESTS})
	get_filename_component(TestName ${test} NAME_WE)
	add_test(${TestName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${TestName})
endforeach()

macro (make_fuzzer name)
    add_executable(${name} ${name}.c)
    set_target_properties(${name} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer" LINK_FLAGS "-fsanitize=fuzzer")
    target_link_libraries(${name} PRIVATE testoutput)
endmacro ()

set(${MODULE_PREFIX}_FUZZERS
		FuzzParseJUnit
		FuzzParseSubunitV1
		FuzzParseSubunitV2
		FuzzParseTestanything)

if(NOT CMAKE_HOST_SYSTEM_NAME MATCHES "OpenBSD" AND CMAKE_C_COMPILER_ID STREQUAL "Clang")
foreach(fuzzer ${${MODULE_PREFIX}_FUZZERS})
	make_fuzzer(${fuzzer})
	get_filename_component(TestName ${fuzzer} NAME_WE)
	add_test(${TestName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${TestName})
endforeach()
endif()
