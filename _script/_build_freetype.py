#!/usr/bin/env python3

import tarfile
import os
import xsetup

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "freetype"
src_file = f"{cwd}/_3rd_source/freetype-2.13.2.tar.gz"
unzipped_src_dir = f"{unzip_dir}/freetype-2.13.2"
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
            f'{xsetup.cmake_build_type} ' \
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DFT_DISABLE_HARFBUZZ=TRUE '
            '-DCMAKE_CXX_STANDARD=20 '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build .')
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
