CMAKE_MINIMUM_REQUIRED (VERSION 2.6)
SET(CMAKE_VERBOSE_MAKEFILE ON)

PROJECT("runtests")

SET(CMAKE_C_FLAGS "-std=c99 -O0 -Wall -Wextra -Wimplicit")

INCLUDE_DIRECTORIES("/usr/local/include")
LINK_DIRECTORIES("/usr/local/lib/")
ADD_EXECUTABLE(runtests testres_tests.c)
TARGET_LINK_LIBRARIES(runtests cunit)

ADD_CUSTOM_TARGET(unit-test COMMAND ${PROJECT_NAME})
