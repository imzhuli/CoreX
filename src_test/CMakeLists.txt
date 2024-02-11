cmake_minimum_required(VERSION 3.14)
project (test_corex)
set(core_libname xel)

include_directories(
	../src
)

include_directories(
	${THIRD_INCLUDE_DIRS}
)

link_directories(
	${THIRD_LIB_DIRS}
)

set(shared_lib_dependency_private
)

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_LIBRARIES ws2_32 Mswsock)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES dl)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	set(PLATFORM_LIBRARIES dl ${IOKIT_LIBRARY} ${COCOA_LIBRARY})
endif()

set(shared_lib_dependency_public
)

if (NOT ${BUILD_SHARED_LIBS}) #static lib:
	set(X_OPTION X_OPTION_STATIC)
endif()

function(build_app sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(_${sub_dir_name} ${LOCAL_SOURCE_CODE})
	target_compile_definitions(_${sub_dir_name} PRIVATE ${X_OPTION})
	target_link_libraries(_${sub_dir_name} PRIVATE ${core_libname})
	target_link_libraries(_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
	target_link_libraries(_${sub_dir_name} PRIVATE ${shared_lib_dependency_public})
endfunction()

function(test_sub sub_dir_name)
	add_test(
		NAME	_${sub_dir_name}
		COMMAND _${sub_dir_name} ${args}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

build_app(cpp)
build_app(echo_client)
build_app(echo_server)
build_app(env_check)
build_app(rsa)

build_app(test_string)
test_sub(test_string)
