set(TARGET winapi_helpers_functional_test)

file(GLOB SOURCES *.cpp)
 
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} PRIVATE Boost::unit_test_framework)
add_test(NAME ${TARGET} COMMAND ${TARGET})