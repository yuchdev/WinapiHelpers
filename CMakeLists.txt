cmake_minimum_required(VERSION 3.0)
project(winapi_helpers CXX)

set(CMAKE_CXX_STANDARD 17)

if(${USE_BOOST})
    set(BOOST_MIN_VERSION "1.74.0")
    set(Boost_ADDITIONAL_VERSIONS "1.74.0" "1.74")
    set(Boost_USE_STATIC_LIBS ON)

    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
endif()

message("CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE})
message("CMAKE_SYSTEM_VERSION="${CMAKE_SYSTEM_VERSION})
message("CMAKE_MODULE_PATH="${CMAKE_MODULE_PATH})

if(${WIN32})
    set(CMAKE_CXX_FLAGS "/EHsc")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
else()
    message(FATAL_ERROR "Non-windows configurations do not supported")
endif()

add_subdirectory(src)

if(${USE_BOOST})
    add_subdirectory(test/functional_test)
endif()
