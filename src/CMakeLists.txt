cmake_minimum_required(VERSION 3.14)
project (libcorex)
set(libname xel)

find_package(Vulkan REQUIRED)

set(THIRD_LIBRARIES     curl everest freetype glfw3 mbedcrypto mbedtls mbedx509 p256m z)

include_directories(
	${Vulkan_INCLUDE_DIRS}
	${THIRD_INCLUDE_DIRS}
)

link_directories(
	${Vulkan_LIBRARY_DIRS}
	${THIRD_LIB_DIRS}
)

set(shared_lib_dependency_private
	${Vulkan_LIBRARIES}
	${THIRD_LIBRARIES}
)
if (${BUILD_SHARED_LIBS})
	set(exposed_lib_dependency ${shared_lib_dependency_private})
	set(shared_lib_dependency_private)
endif()

# enable specific macors by system type:
if (CMAKE_SYSTEM_NAME MATCHES "Windows")
	set(PLATFORM_LIBRARIES ws2_32 Mswsock)
elseif (CMAKE_SYSTEM_NAME MATCHES "Linux")
	set(PLATFORM_LIBRARIES dl)
elseif (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	find_library(COCOA_LIBRARY Cocoa REQUIRED)
	find_library(IOKIT_LIBRARY IOKit REQUIRED)
	find_library(FUNDATION_LIBRARY Foundation REQUIRED)
	find_library(SYSTEM_CONFIGURATION_LIBRARY SystemConfiguration REQUIRED)
	set(PLATFORM_LIBRARIES dl ${SYSTEM_CONFIGURATION_LIBRARY} ${FUNDATION_LIBRARY} ${IOKIT_LIBRARY} ${COCOA_LIBRARY} )
endif()

set(shared_lib_dependency_public
	${exposed_lib_dependency}
	${PLATFORM_LIBRARIES}
	${cpp})


aux_source_directory(./algorithm/ X_SRC)
aux_source_directory(./core/          X_SRC)
aux_source_directory(./crypto/        X_SRC)
aux_source_directory(./locale/        X_SRC)
aux_source_directory(./network/       X_SRC)
aux_source_directory(./object/        X_SRC)
aux_source_directory(./server_arch/   X_SRC)

if (NOT ${SERVER_SIDE_LIB_ONLY})
	aux_source_directory(./renderer/      X_SRC)
	aux_source_directory(./vk/            X_SRC)
	aux_source_directory(./wsi/           X_SRC)
	aux_source_directory(./xengine/       X_SRC)

	if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
	aux_source_directory(./wsi/darwin  X_SRC)
	endif()

	# add 3rd lib files
	aux_source_directory(./3rd/stb_image         X_3RD_SRC)
	aux_source_directory(./3rd/tiny_obj_loader   X_3RD_SRC)
	aux_source_directory(./3rd/vk_bootstrap      X_3RD_SRC)
	aux_source_directory(./3rd/vk_mem_alloc      X_3RD_SRC)
endif()

if (NOT ${BUILD_SHARED_LIBS}) #static lib:
set(X_OPTION X_OPTION_STATIC)
else() #dynamic lib:
set(X_OPTION X_OPTION_EXPORT_API)
endif()

add_library(${libname} ${X_SRC} ${X_3RD_SRC})
target_compile_definitions(${libname} PRIVATE ${X_OPTION})
set_target_properties(${libname} PROPERTIES VERSION 1.0)
target_link_libraries(${libname} PRIVATE ${shared_lib_dependency_private})
target_link_libraries(${libname} PUBLIC ${shared_lib_dependency_public})

install(TARGETS
	${libname}
	RUNTIME DESTINATION bin
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	)

