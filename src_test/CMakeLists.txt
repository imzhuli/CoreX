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
	${THIRD_LIBRARIES}
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
build_app(rsa)
build_app(timer_wheel)
build_app(object_id_manager)

build_app(mp)

if (NOT ${SERVER_SIDE_LIB_ONLY})
build_app(env_check)
target_link_options(_env_check PRIVATE "" ${RELEASE_LINK_OPTIONS_NO_CONSOLE})
endif()

