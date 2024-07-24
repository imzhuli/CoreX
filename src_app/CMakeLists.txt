cmake_minimum_required(VERSION 3.14)
project (app_corex)
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
	add_executable(x_${sub_dir_name} ${LOCAL_SOURCE_CODE})
	target_compile_definitions(x_${sub_dir_name} PRIVATE ${X_OPTION})
	target_link_libraries(x_${sub_dir_name} PRIVATE ${core_libname})
	target_link_libraries(x_${sub_dir_name} PRIVATE ${shared_lib_dependency_private})
endfunction()

build_app(abort)
build_app(broadcaster)
build_app(dispatcher)
build_app(guard)

install(TARGETS
	x_abort
	x_broadcaster
	x_dispatcher
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)
