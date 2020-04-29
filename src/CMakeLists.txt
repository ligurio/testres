cmake_minimum_required(VERSION 3.9.2)

set(SOURCE_FILES
metrics.c
ui_common.c
ui_console.c
ui_http.c
)

include(FindEXPAT)
find_package(EXPAT REQUIRED)

include_directories(${EXPAT_INCLUDE_DIRS} "../libtestoutput")

set(LIBS testoutput m ${EXPAT_LIBRARIES})
add_executable(${PROJECT_NAME} testres.c ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${LIBS})

if(ENABLE_STATIC_BUILD)
    set(LIBS ${LIBS} -static)
endif()
add_executable(${PROJECT_NAME}.cgi testres.cgi.c ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}.cgi ${LIBS})
