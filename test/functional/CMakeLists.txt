set(TARGET winapi_helpers_functional_test)

file(GLOB SOURCES win_helpers_func_tests.cpp)
find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS unit_test_framework system filesystem thread REQUIRED)
 
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

add_executable(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} 
PRIVATE 
    ${Boost_LIBRARIES}
    winapi_helpers
)
add_test(NAME ${TARGET} COMMAND ${TARGET})
set_property(TARGET ${TARGET} PROPERTY FOLDER "UnitTests")
