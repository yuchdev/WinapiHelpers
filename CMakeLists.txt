# ---- Project header ----
cmake_minimum_required(VERSION 3.20)
set(WINAPI_HELPERS_TARGET winapi_helpers)
project(${WINAPI_HELPERS_TARGET} CXX)

message("BUILD_SHARED_LIBS=" ${BUILD_SHARED_LIBS})
message("CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE})
message("CMAKE_SYSTEM_VERSION=" ${CMAKE_SYSTEM_VERSION})

# ---- Frameworks and components ----
if(${BOOST_ROOT})
    set(BOOST_MIN_VERSION "1.74.0")
    set(Boost_ADDITIONAL_VERSIONS "1.74.0" "1.74")
    set(Boost_USE_STATIC_LIBS ON)
    find_package(Boost COMPONENTS unit_test_framework REQUIRED)
endif()

# ---- Target ----
# Check if building a static or shared library
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

# Load SOURCES
include(sources.cmake)

# Create the library target
add_library(${WINAPI_HELPERS_TARGET} ${SOURCES})

# ---- Global properties ----

# ---- Cross-compiler options ----
# Set C++17 as the standard
target_compile_features(${WINAPI_HELPERS_TARGET} PUBLIC cxx_std_17)

# ---- System-specific options ----
# Set Exception handling as exceptions, suppress MSVC security warnings, and use Visual Studio Folders
if(WIN32)
    set(CMAKE_CXX_FLAGS "/EHsc")
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)    
else()
    message(FATAL_ERROR "Non-windows configurations does not supported")
endif()

# ---- Include directories ----
target_include_directories(${WINAPI_HELPERS_TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_SOURCE_DIR}/include"
)

# ---- Library dependencies ----
target_link_libraries(${WINAPI_HELPERS_TARGET} Secur32)

# ---- Enable testing ----
# We use Boost Test, so include it only if Boost root is known
if(${BOOST_ROOT})
    add_subdirectory(test/functional_test)
endif()

# ---- Additional build steps ----

# ---- Install rules ----
