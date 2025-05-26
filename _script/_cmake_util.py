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

def __compare_cmake_versions(version1, version2):
    def normalize(v):
        return [int(x) for x in v.split('.')]
    v1 = normalize(version1)
    v2 = normalize(version2)
    for i in range(max(len(v1), len(v2))):
        n1 = v1[i] if i < len(v1) else 0
        n2 = v2[i] if i < len(v2) else 0
        if n1 < n2:
            return -1
        elif n1 > n2:
            return 1
    return 0

def ensure_cmake_minimum_required(cmake_file, version="3.10"):
    try:
        # 读取文件内容
        with open(cmake_file, 'r', encoding='utf-8') as f:
            lines = f.readlines()
        
        # 检查是否已存在cmake_minimum_required命令
        pattern = re.compile(r'^\s*cmake_minimum_required\s*\(')
        has_cmake_min_req = any(pattern.match(line.strip()) for line in lines)
        
        if not has_cmake_min_req:
            new_line = f"cmake_minimum_required(VERSION {version})\n"
            lines.insert(0, new_line)
            with open(cmake_file, 'w', encoding='utf-8') as f:
                f.writelines(lines)
            print(f"cmake_minimum_required added to {cmake_file}")
        return True
    except Exception as e:
        print(f"error processing {cmake_file}: {e}")
        return False

def update_cmake_minimum_required(cmake_file, min_version="3.5", new_version="3.10"):
    no_cmake_minimum_required = False
    try:
        with open(cmake_file, 'r', encoding='utf-8') as f:
            content = f.read()
        
        pattern = re.compile(r'cmake_minimum_required\s*\(\s*VERSION\s+["\']?(\d+\.\d+)["\']?\s*\)')
        match = pattern.search(content)
        
        if match:
            current_version = match.group(1)
            if compare_versions(current_version, min_version) <= 0:
                updated_content = pattern.sub(
                    f'cmake_minimum_required(VERSION "{new_version}")',
                    content
                )
                if updated_content != content:
                    with open(cmake_file, 'w', encoding='utf-8') as f:
                        f.write(updated_content)
                    print(f"updated {cmake_file} cmake_minimum_required from {current_version} to {new_version}")
        else:
            # cmake_minimum_required not found:
            no_cmake_minimum_required = True
    except Exception as e:
        print(f"error on processing {cmake_file}: {e}")
        return False
    ensure_cmake_minimum_required(cmake_file, new_version)
    return True


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
