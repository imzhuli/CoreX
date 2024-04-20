#!/usr/bin/env python3

import tarfile
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "zlib"
src_file = f"{cwd}/_3rd_source/zlib-1.3.1.tar.gz"
unzipped_src_dir = f"{unzip_dir}/zlib-1.3.1"
install_dir = f"{cwd}/_3rd_installed"

def disable_installing_shared_lib():
    cmakefile = f"{unzipped_src_dir}/CMakeLists.txt"
    try:
        with open(cmakefile, "r") as sources:
            lines = sources.readlines()
        with open(cmakefile, "w") as sources:
            for line in lines:
                sources.write(line.replace(
                    'TARGETS zlib zlibstatic', 'TARGETS zlibstatic'))
    except Exception as e:
        print("disable_installing_shared_lib error: %s" % e)
        return False
    return True


def build():
    if os.getenv("PS_BUILD_CONFIG_TYPE") is None:
        os.environ["PS_BUILD_CONFIG_TYPE"]="Debug"
    build_type=os.getenv("PS_BUILD_CONFIG_TYPE")
    print(f"=============> {build_type}")

    try:
        file = tarfile.open(src_file)
        file.extractall(unzip_dir)
    finally:
        file.close()

    if not disable_installing_shared_lib():
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DZLIB_BUILD_EXAMPLES=OFF '
            '-DCMAKE_CXX_STANDARD=20 '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build . ')
        os.system(f"cmake --build build --config {build_type}")
        os.system(f"cmake --install build --config {build_type}")
    except Exception as e:
        print(f"{libname} error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
