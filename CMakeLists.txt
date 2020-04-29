cmake_minimum_required (VERSION 3.9.2)
project(testres)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Wfloat-equal")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wundef -Wpointer-arith -Wcast-align -Wshadow")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wswitch-enum -Winit-self")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Werror -pedantic")

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O3")
if (NOT FreeBSD)
	set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fsanitize=address")
endif (NOT FreeBSD)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O0 -g")

include(CTest)

option(ENABLE_FUZZER "Enable fuzzing testing" OFF)

if(BUILD_TESTING)
        enable_testing()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-function -Wno-unused-variable")
        set(TESTING_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Testing")
endif()

add_subdirectory(libtestoutput)
add_subdirectory(src)
