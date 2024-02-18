#!/usr/bin/env python3

import tarfile
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "rapidjson"
src_file = f"{cwd}/_3rd_source/rapidjson-1.1.0.tar.gz"
unzipped_src_dir = f"{unzip_dir}/rapidjson-1.1.0"
install_dir = f"{cwd}/_3rd_installed"

cpp_flags = """
-Wno-deprecated-declarations
-Wno-implicit-fallthrough
-Wno-shadow 
-Wno-suggest-override
-Wno-suggest-destructor-override
-Wno-zero-as-null-pointer-constant
""".replace("\n", " ")


def fix_cmake():
    cmakefile = f"{unzipped_src_dir}/CMakeLists.txt"
    find_target = """if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")"""
    replace_target = """    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess")\n"""
    try:
        with open(cmakefile, "r") as sources:
            lines = sources.readlines()
        with open(cmakefile, "w") as sources:
            for line in lines:
                sources.write(line)
                if 0 == line.find(find_target):
                    sources.write(replace_target)
    except Exception as e:
        return False
    return True


def build():
    try:
        file = tarfile.open(src_file)
        file.extractall(unzip_dir)
    finally:
        file.close()

    if not fix_cmake():
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev '
            f'-DCMAKE_CXX_FLAGS="{cpp_flags}" '
            f'-DCMAKE_INSTALL_PREFIX={install_dir!r} -B build . ')
        os.system(f"cmake --build build -- all")
        os.system(f"cmake --build build -- install")
    except Exception as e:
        print(f"{libname} error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    print(f"{libname} installed to {install_dir!r}")
    return True


if __name__ == "__main__":
    build()
