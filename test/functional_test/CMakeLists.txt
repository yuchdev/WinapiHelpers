set(TARGET winapi_helpers_functional_test)

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS unit_test_framework date_time REQUIRED) 

set(DEPENDENT_LIBRARIES winapi_helpers)

file(GLOB SOURCES *.cpp)
 
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

add_executable(${TARGET} ${SOURCES})
add_dependencies(${TARGET} ${DEPENDENT_LIBRARIES})
target_link_libraries(${TARGET} ${Boost_LIBRARIES} ${DEPENDENT_LIBRARIES})
add_test(NAME ${TARGET} COMMAND ${TARGET})