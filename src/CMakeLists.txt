set(TARGET winapi_helpers)
 
file(GLOB SOURCES *.cpp ../include/${TARGET}/*.h)
 
include_directories(
    ${CMAKE_SOURCE_DIR}/include)
 
add_library(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} Secur32)
