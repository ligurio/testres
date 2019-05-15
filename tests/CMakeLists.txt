cmake_minimum_required (VERSION 3.10)

set(CMAKE_C_FLAGS "-std=c99 -O0 -Wall -Wextra -Wimplicit")

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${${PROJECT_NAME}_SOURCE_DIR}/src
					${EXPAT_INCLUDE_DIRS}
					"/usr/local/include")
link_directories("/usr/local/lib/")
add_executable(${PROJECT_NAME}-tests testres_tests.c)
target_link_libraries(${PROJECT_NAME}-tests
					${EXPAT_LIBRARIES}
					${PROJECT_NAME}-lib
					expat
					cunit
					m)
