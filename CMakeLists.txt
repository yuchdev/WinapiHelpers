cmake_minimum_required(VERSION 3.18)
project(winapi_helpers CXX)

if(${BOOST_ROOT})
    set(BOOST_MIN_VERSION "1.74.0")
    set(Boost_ADDITIONAL_VERSIONS "1.74.0" "1.74")
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
endif()

message("BUILD_SHARED_LIBS=" ${BUILD_SHARED_LIBS})
message("CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE})
message("CMAKE_SYSTEM_VERSION=" ${CMAKE_SYSTEM_VERSION})

# Check if building a static or shared library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Create the library target
include(sources.cmake)
add_library(winapi_helpers ${SOURCES})

# Set public include directories
target_include_directories(winapi_helpers
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

# Set C++17 as the standard
target_compile_features(winapi_helpers PUBLIC cxx_std_17)

# Set Windows-specific cinfiguration and dependencies
if(WIN32)
    set(CMAKE_CXX_FLAGS "/EHsc")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
    target_link_libraries(winapi_helpers Secur32)
else()
    message(FATAL_ERROR "Non-windows configurations do not supported")
endif()

# We use Boost Test, so include it only if Boost root is known
if(${BOOST_ROOT})
    add_subdirectory(test/functional_test)
endif()
