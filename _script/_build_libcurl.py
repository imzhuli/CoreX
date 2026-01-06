#!/usr/bin/env python3

import _cmake_util as cu
import tarfile
import os
import xsetup

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "curl"
src_file = f"{cwd}/_3rd_source/curl-8.6.0.tar.gz"
unzipped_src_dir = f"{unzip_dir}/curl-8.6.0"
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
            "cmake "
            f"{xsetup.cmake_build_type} "
            "-Wno-dev "
            "-DBUILD_SHARED_LIBS=OFF "
            "-DBUILD_LIBCURL_DOCS=OFF "
            "-DCURL_USE_MBEDTLS=ON "
            "-BUILD_CURL_EXE=OFF "
            f"-DMBEDTLS_INCLUDE_DIRS={install_dir}/include/ "
            ## f"-DMBEDTLS_LIBRARY={install_dir}/lib/libmbedtls.a "
            ## f"-DMBEDX509_LIBRARY={install_dir}/lib/libmbedx509.a "
            ## f"-DMBEDCRYPTO_LIBRARY={install_dir}/lib/libmbedcrypto.a "
            "-DCURL_DISABLE_LDAP=ON "
            "-DPICKY_COMPILER=OFF "
            "-DCMAKE_CXX_STANDARD=20 "
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build . '
        )
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
