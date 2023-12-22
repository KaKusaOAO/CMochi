cmake_minimum_required(VERSION 3.27)
project(Mochi)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/install)

include_directories(include)
aux_source_directory(src MOCHI_SRC)
aux_source_directory(test MOCHI_TEST)

include(FetchContent)

fetchcontent_declare(
        jsoncpp
        GIT_REPOSITORY https://github.com/KaKusaOAO/jsoncpp
        GIT_TAG master
)

fetchcontent_declare(
        SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG main
)

fetchcontent_makeavailable(jsoncpp SDL)

link_libraries(jsoncpp_lib SDL3-shared)

add_library(Mochi-static STATIC ${MOCHI_SRC} )
add_library(Mochi-shared SHARED ${MOCHI_SRC} )
add_executable(Mochi-test ${MOCHI_SRC} ${MOCHI_TEST} )

set_target_properties(Mochi-static Mochi-shared
        PROPERTIES OUTPUT_NAME Mochi)
