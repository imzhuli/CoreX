#!/usr/bin/env python3

import os
import re
import shutil
import stat

__fix_compiler_flags = """
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nested-externs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-conversion")
elseif (CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-implicit-function-declaration")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-nested-externs")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-int-conversion")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-sign-conversion")
elseif (CMAKE_C_COMPILER_ID MATCHES "MSVC")
	add_compile_options("-wd4267")
	add_compile_options("-wd4244")
	add_compile_options("-wd4819")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # GNU only begin
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")
    # GNU only end

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-fallthrough")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-destructor-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-#pragma-messages")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-int-conversion")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-utf8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-fallthrough")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-suggest-destructor-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-zero-as-null-pointer-constant")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-#pragma-messages")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-implicit-int-conversion")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-invalid-utf8")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-but-set-variable")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()
"""


def __remove_readonly(func, path, _):
    try:
        os.chmod(path, stat.S_IWRITE | stat.S_IREAD)
        func(path)
    except Exception as e:
        print(path)
        print(e)


def fix_cmake(cmakefile):
    try:
        find_target = r"^PROJECT\(.+\)"
        with open(cmakefile, "r") as sources:
            lines = sources.readlines()
        with open(cmakefile, "w") as sources:
            for line in lines:
                sources.write(line)
                if re.search(find_target, line, re.IGNORECASE):
                    sources.write(__fix_compiler_flags)
    except Exception as e:
        return False
    return True


def remove_dir(dir):
    # try:
        shutil.rmtree(dir, onerror=__remove_readonly)
    # except Exception as e:
    #     print(f"RM: Exception: {e}")


def remake_dir(dir):
    remove_dir(dir)
    try:
        os.makedirs(dir)
    except Exception as e:
        print(f"MK: Exception: {e}")
        return False
    return True
