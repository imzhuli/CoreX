#!/usr/bin/env python3

import _unzip_source as us
import _cmake_util as cu
import os
import xsetup

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "mbedtls"
src_file = f"{cwd}/_3rd_source/mbedtls-3.5.2.zip"
unzipped_src_dir = f"{unzip_dir}/mbedtls-3.5.2"
install_dir = f"{cwd}/_3rd_installed"


def build():
    if not us.unzip_source(unzip_dir, src_file):
        print("failed to unzip source: %s" % src_file)
        return False

    cmake_file = f"{unzipped_src_dir}/CMakeLists.txt"
    if not cu.fix_cmake(cmake_file):
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            f'{xsetup.cmake_build_type} ' \
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DENABLE_TESTING=OFF '
            '-DCMAKE_CXX_STANDARD=20 '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build . ')
        os.system(f"cmake --build build {xsetup.cmake_build_config}")
        os.system(f"cmake --install build {xsetup.cmake_build_config}")
    except Exception as e:
        print(f"{libname} error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
