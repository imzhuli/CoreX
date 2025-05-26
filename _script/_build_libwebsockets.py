#!/usr/bin/env python3

import tarfile
import os
import _cmake_util as cu
import xsetup

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

src_file = f"{cwd}/_3rd_source/libwebsockets-4.3.3.tar.gz"
unzipped_src_dir = f"{unzip_dir}/libwebsockets-4.3.3"
install_dir = f"{cwd}/_3rd_installed"

def build():
    try:
        file = tarfile.open(src_file)
        file.extractall(unzip_dir)
    finally:
        file.close()

    cmake_file = f"{unzipped_src_dir}/CMakeLists.txt"
    if not cu.fix_cmake(cmake_file):
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            f'{xsetup.cmake_build_type} ' \
            '-Wno-dev '
            '-DLWS_WITH_MBEDTLS=ON '
            '-DLWS_WITH_SHARED=OFF '
            '-DLWS_WITHOUT_TESTAPPS=ON '
            '-DLWS_WITHOUT_TEST_SERVER=ON '
            '-DLWS_WITHOUT_TEST_SERVER_EXTPOLL=ON '
            '-DLWS_WITHOUT_TEST_PING=ON '
            '-DLWS_WITHOUT_TEST_CLIENT=ON '
            '-DBUILD_TESTING=OFF '
            '-DLWS_HAVE_MBEDTLS_NET_SOCKETS=1 '
            '-DCMAKE_CXX_STANDARD=20 '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build .')
        os.system(f"cmake --build build {xsetup.cmake_build_config}")
        os.system(f"cmake --install build {xsetup.cmake_build_config}")
    except Exception as e:
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
