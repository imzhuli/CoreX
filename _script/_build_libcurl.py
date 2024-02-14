#!/usr/bin/env python3

import tarfile
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "curl"
src_file = f"{cwd}/_3rd_source/curl-8.6.0.tar.gz"
unzipped_src_dir = f"{unzip_dir}/curl-8.6.0"
install_dir = f"{cwd}/_3rd_installed"

c_flags = """
-Wno-implicit-function-declaration
-Wno-nested-externs
-Wno-int-conversion
""".replace("\n", " ")

def build():
    try:
        file = tarfile.open(src_file)
        file.extractall(unzip_dir)
    finally:
        file.close()

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev -DBUILD_SHARED_LIBS=OFF '
            '-DBUILD_LIBCURL_DOCS=OFF '
            '-DCURL_USE_MBEDTLS=ON '
            '-DCURL_DISABLE_LDAP=ON '
            f'-DCMAKE_C_FLAGS="{c_flags}" '
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
