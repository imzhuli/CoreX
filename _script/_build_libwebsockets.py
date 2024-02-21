#!/usr/bin/env python3

import tarfile
import os

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

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DLWS_WITH_SHARED=OFF '
            '-DLWS_WITHOUT_TESTAPPS=ON '
            '-DLWS_WITHOUT_TEST_SERVER=ON '
            '-DLWS_WITHOUT_TEST_SERVER_EXTPOLL=ON '
            '-DLWS_WITHOUT_TEST_PING=ON '
            '-DLWS_WITHOUT_TEST_CLIENT=ON '
            '-DBUILD_TESTING=OFF '
            f'-DCMAKE_INSTALL_PREFIX={install_dir!r} -B build .')
        os.system(f"cmake --build build -- all")
        os.system(f"cmake --build build -- install")
    except Exception as e:
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
