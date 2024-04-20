#!/usr/bin/env python3

import _cmake_util as cu
import _unzip_source as us
import os
import stat
import shutil

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "glm"
src_file = f"{cwd}/_3rd_source/glm-1.0.0-light.zip"
unzipped_src_dir = f"{unzip_dir}/glm"
install_dir = f"{cwd}/_3rd_installed"

def build():
    if os.getenv("PS_BUILD_CONFIG_TYPE") is None:
        os.environ["PS_BUILD_CONFIG_TYPE"]="Debug"
    build_type=os.getenv("PS_BUILD_CONFIG_TYPE")
    print(f"=============> {build_type}")

    if not us.unzip_source(unzip_dir, src_file):
        print("failed to unzip source: %s" % src_file)
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            f'-DCMAKE_INSTALL_PREFIX="{install_dir}" -B build . ')
        os.system(f"cmake --build build --config {build_type}")
        os.system(f"cmake --install build --config {build_type}")
    except Exception as e:
        print(f"{libname} error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    return True


def post_build():
    try:
        src_include_dir = unzipped_src_dir
        target_include_dir = install_dir + "/include/glm"
        cu.remove_dir(unzipped_src_dir + "/build")
        cu.remove_dir(target_include_dir)
        shutil.copytree(src_include_dir, target_include_dir)
    except Exception as e:
        print(f"Exception: {e}")
        return False
    return True


if __name__ == "__main__":
    if build():
        post_build()
