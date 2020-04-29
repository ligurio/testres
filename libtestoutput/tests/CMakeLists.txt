cmake_minimum_required (VERSION 3.9.2)

set(MODULE_NAME "libtestoutput")
set(MODULE_PREFIX "TEST_LIB_TEST_OUTPUT")

#set(${MODULE_PREFIX}_DRIVER ${MODULE_NAME}.c)

set(${MODULE_PREFIX}_TESTS
		TestParseJUnit.c
		TestParseSubunitV1.c
		TestParseSubunitV2.c
		TestParseTestanything.c)

include_directories("${CMAKE_SOURCE_DIR}/libtestoutput")

foreach(test ${${MODULE_PREFIX}_TESTS})
	get_filename_component(TestName ${test} NAME_WE)
	set(${TestName}_DRIVER "main_${TestName}.c")
	create_test_sourcelist(${TestName}_SRCS ${${TestName}_DRIVER} ${test})
	add_executable(${TestName} ${${TestName}_SRCS})
	target_link_libraries(${TestName} testoutput ${EXPAT_LIBRARIES} m)
	add_test(${TestName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${TestName})
endforeach()

set(${MODULE_PREFIX}_FUZZERS
		FuzzParseJUnit.c
		FuzzParseSubunitV1.c
		FuzzParseSubunitV2.c
		FuzzParseTestanything.c)

if((NOT CMAKE_HOST_SYSTEM_NAME MATCHES "OpenBSD") AND (CMAKE_C_COMPILER_ID STREQUAL "Clang"))
foreach(fuzzer ${${MODULE_PREFIX}_FUZZERS})
	get_filename_component(FuzzerName ${fuzzer} NAME_WE)
	add_executable(${FuzzerName} ${fuzzer})
	set_target_properties(${FuzzerName} PROPERTIES COMPILE_FLAGS "-fsanitize=fuzzer" LINK_FLAGS "-fsanitize=fuzzer")
	target_link_libraries(${FuzzerName} PRIVATE testoutput)
	add_test(${FuzzerName} ${TESTING_OUTPUT_DIRECTORY}/${MODULE_NAME} ${FuzzerName})
endforeach()
endif()
