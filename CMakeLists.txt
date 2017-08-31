cmake_minimum_required(VERSION 3.4)

project(server)

#----------------------- compile options ------------------------
set(CMAKE_CXX_STANDARD 14)

# ---------------------------- boost ----------------------------
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT C:/libs/boost_1_62_0)
set(BOOST_LIBRARYDIR C:/libs/boost_1_62_0/lib32-msvc-14.0)
find_package(Boost COMPONENTS
    date_time
    regex
    system
    REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

# -------------------------- threads ----------------------------
find_package(Threads)

# --------------------------- sources ---------------------------
set(HEADERS
    src/rapidjson/encodings.h
    src/rapidjson/istreamwrapper.h
    src/rapidjson/reader.h
    src/rapidjson/stringbuffer.h
    src/rapidjson/writer.h

    src/data_base.h
    src/handler.h
    src/http_parser.h
    src/http_server.h
    src/in_place_writer.h
    src/json.h
    src/responses.h
    src/types.h
    src/url_parser.h
    src/utils.h
    )

set(SOURCES
    src/data_base.cpp
    src/handler.cpp
    src/http_parser.cpp
    src/http_server.cpp
    src/in_place_writer.cpp
    src/json.cpp
    src/responses.cpp
    src/url_parser.cpp
    src/utils.cpp

    src/main.cpp)

include_directories(./src)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

target_link_libraries (${PROJECT_NAME}
    ${Boost_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )

set(BUILD_TESTS ON)

if(BUILD_TESTS)
    add_executable(tests tests/test_url_parser.cpp
        src/http_parser.cpp
        src/in_place_writer.cpp
        src/json.cpp
        src/url_parser.cpp
        src/utils.cpp)

    set_target_properties(tests PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

    target_link_libraries (tests
        ${Boost_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )
endif()

