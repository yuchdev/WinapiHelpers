set(TARGET winapi_helpers)

find_package(Boost ${BOOST_MIN_VERSION} COMPONENTS thread chrono filesystem serialization regex system date_time REQUIRED)
 
file(GLOB SOURCES *.cpp ../include/${TARGET}/*.h)
 
include_directories(
    ${Boost_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}/../include)
 
add_library(${TARGET} ${SOURCES})
target_link_libraries(${TARGET} ${Boost_LIBRARIES} Secur32)
