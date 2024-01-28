cmake_minimum_required(VERSION 3.14)
project (libcorex)
set(libname xel)

include_directories(
	../src
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
	${PLATFORM_LIBRARIES}
	${cpp})

aux_source_directory(./_/ SHARED_SRC)

function(build_shared sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_library(_${sub_dir_name}  ${LOCAL_SOURCE_CODE} ${SHARED_SRC})
	target_link_libraries(_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
endfunction()

function(build_app sub_dir_name)
	aux_source_directory(${sub_dir_name} LOCAL_SOURCE_CODE)
	add_executable(_${sub_dir_name}  ${LOCAL_SOURCE_CODE} ${SHARED_SRC})
	target_link_libraries(_${sub_dir_name} PRIVATE ${libname})
	target_link_libraries(_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
if (X_OPTION_STATIC)
	target_compile_definitions(_${sub_dir_name} PRIVATE X_OPTION_STATIC)
endif()
endfunction()

function(test_sub sub_dir_name)
	add_test(
		NAME	_${sub_dir_name}
		COMMAND _${sub_dir_name} ${args}
		WORKING_DIRECTORY  ${CMAKE_SOURCE_DIR}
		)
endfunction()

#

build_app(cpp)
build_app(env_check)
build_app(string)

test_sub(string)
