cmake_minimum_required(VERSION 3.9.2)

set(SOURCE_FILES
testres.c
metrics.c
ui_common.c
ui_console.c
ui_http.c
)

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${EXPAT_INCLUDE_DIRS} "../libtestoutput")
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} testoutput ${EXPAT_LIBRARIES} m)
