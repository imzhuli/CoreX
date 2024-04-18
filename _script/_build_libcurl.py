#!/usr/bin/env python3

import _cmake_util as cu
import tarfile
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "curl"
src_file = f"{cwd}/_3rd_source/curl-8.6.0.tar.gz"
unzipped_src_dir = f"{unzip_dir}/curl-8.6.0"
install_dir = f"{cwd}/_3rd_installed"

def build():
    if os.getenv("CMAKE_BUILD_TYPE") is None:
        os.environ["CMAKE_BUILD_TYPE"]="Debug"
    build_type=os.getenv("CMAKE_BUILD_TYPE")
    print(f"=============> {build_type}")

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
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DBUILD_LIBCURL_DOCS=OFF '
            '-DCURL_USE_MBEDTLS=ON '
            '-DCURL_DISABLE_LDAP=ON '
            '-DPICKY_COMPILER=OFF '
            '-DCMAKE_CXX_STANDARD=20 '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build . ')
        os.system(f"cmake --build build --config={build_type}")
        os.system(f"cmake --install build --config={build_type}")
    except Exception as e:
        print(f"{libname} error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
