#!/usr/bin/env python3

import _unzip_source as us
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "glfw3"
src_file = f"{cwd}/_3rd_source/glfw-3.3.9.zip"
unzipped_src_dir = f"{unzip_dir}/glfw-3.3.9"
install_dir = f"{cwd}/_3rd_installed"


def build():
    if not us.unzip_source(unzip_dir, src_file):
        print("failed to unzip source: %s" % src_file)
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            f'cmake -Wno-dev -DBUILD_SHARED_LIBS=OFF -DCMAKE_INSTALL_PREFIX={install_dir!r} -B build .')
        os.system(f"cmake --build build -- all")
        os.system(f"cmake --build build -- install")
    except Exception as e:
        print(f"{libname} build error: %s" % e)
        return False
    finally:
        os.chdir(cwd)
    print(f"{libname} installed to {install_dir!r}")
    return True


if __name__ == "__main__":
    build()
