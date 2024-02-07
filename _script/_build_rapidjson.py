#!/usr/bin/env python3

import tarfile
import os

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "rapidjson"
src_file = f"{cwd}/_3rd_source/rapidjson-1.1.0.tar.gz"
unzipped_src_dir = f"{unzip_dir}/rapidjson-1.1.0"
install_dir = f"{cwd}/_3rd_installed"

cpp_flags = """
-Wno-zero-as-null-pointer-constant
-Wno-deprecated-declarations
-Wno-shadow 
-Wno-suggest-override
-Wno-suggest-destructor-override
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
            '-Wno-dev '
            '-DBUILD_SHARED_LIBS=OFF '
            f'-DCMAKE_CXX_FLAGS="{cpp_flags}" '
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
