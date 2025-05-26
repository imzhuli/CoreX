#!/usr/bin/env python3

import _unzip_source as us
import os
import xsetup

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "glfw3"
src_file = f"{cwd}/_3rd_source/glfw-3.4.zip"
unzipped_src_dir = f"{unzip_dir}/glfw-3.4"
install_dir = f"{cwd}/_3rd_installed"

def build():
    if not us.unzip_source(unzip_dir, src_file):
        print("failed to unzip source: %s" % src_file)
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            f'{xsetup.cmake_build_type} ' \
            '-Wno-dev '
            '-DCMAKE_POLICY_VERSION_MINIMUM=3.5 '
            '-DGLFW_BUILD_DOCS=OFF '
            '-DBUILD_SHARED_LIBS=OFF '
            '-DGLFW_BUILD_WAYLAND=OFF '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build .')
        os.system(f"cmake --build build {xsetup.cmake_build_config}")
        os.system(f"cmake --install build {xsetup.cmake_build_config}")
    except Exception as e:
        print(f"{libname} build error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    return True


if __name__ == "__main__":
    build()
