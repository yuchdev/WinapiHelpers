cmake_minimum_required(VERSION 3.0)
project(winapi_helpers CXX)

set(CMAKE_CXX_STANDARD 17)

set(BOOST_MIN_VERSION "1.61.0")
set(Boost_ADDITIONAL_VERSIONS "1.61.0" "1.61")
set(Boost_USE_STATIC_LIBS ON)
set(BOOST_ROOT ${MY_BOOST_DIR})

find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)

if(${WIN32})
    set(CMAKE_CXX_FLAGS "/EHsc")
    set(MY_BOOST_DIR ${WINDOWS_BOOST_DIR})
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    message(FATAL_ERROR "Non-windows configurations do not supported")
endif()

add_subdirectory(src)
add_subdirectory(test/functional_test)
