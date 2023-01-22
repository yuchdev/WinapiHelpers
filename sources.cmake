set(WINAPI_HELPERS_CPP
  ${CMAKE_CURRENT_SOURCE_DIR}/src/bios.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/co_initializer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/one_instance.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/partition_information.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/physical_memory.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/process_helper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/registry_helper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/service_helper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite3_helper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/special_path_helper.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/system_information.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/user_information.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/utilities.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/win_partition_information.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/win_special_path_helper.cpp
)

set(WINAPI_HELPERS_H
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/utilities.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/bios.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/co_initializer.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/dynamic_handler_map.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/md5.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/win_errors.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/handle_ptr.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/hardware_information.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/native_api_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/one_instance.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/partition_information.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/win_partition_information.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/physical_memory.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/process_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/ptrs.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/registry_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/service_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/special_path_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/win_special_path_helper.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/system_information.h
	${CMAKE_CURRENT_SOURCE_DIR}/include/winapi-helpers/user_information.h
)
set(WINAPI_HELPERS_SOURCES 
	${WINAPI_HELPERS_CPP} 
	${WINAPI_HELPERS_H}
)
