cmake_minimum_required(VERSION 3.27)
project(Mochi)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

include_directories(include)
aux_source_directory(src MOCHI_SRC)
aux_source_directory(test MOCHI_TEST)

include(externalproject)

externalproject_add(PROJECT_JSON
        URL 3rdparty/jsoncpp
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/jsoncpp
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

externalproject_add(PROJECT_SDL
        URL 3rdparty/SDL
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/3rdparty/SDL
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
)

include_directories(${CMAKE_INSTALL_PREFIX}/include)

file(GLOB MOCHI_LINKS ${CMAKE_INSTALL_PREFIX}/lib/*.a)
link_libraries(${MOCHI_LINKS})

add_library(MochiStatic STATIC ${MOCHI_SRC})
add_library(MochiShared SHARED ${MOCHI_SRC})
add_executable(MochiTest ${MOCHI_SRC} ${MOCHI_TEST})

set_target_properties(MochiStatic MochiShared
        PROPERTIES OUTPUT_NAME Mochi)

add_dependencies(MochiStatic PROJECT_JSON PROJECT_SDL)
add_dependencies(MochiShared PROJECT_JSON PROJECT_SDL)
add_dependencies(MochiTest PROJECT_JSON PROJECT_SDL)

install(TARGETS MochiStatic MochiShared
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_PREFIX}/include
)
