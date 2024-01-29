import _unzip_source as us
import os
import shutil

cwd = os.getcwd()
unzip_dir = f"{cwd}/_3rd_build"

libname = "mbedtls"
src_file = f"{cwd}/_3rd_source/mbedtls-3.5.2.zip"
unzipped_src_dir = f"{unzip_dir}/mbedtls-3.5.2"
install_dir = f"{cwd}/_3rd_installed"


def build():
    if not us.unzip_source(unzip_dir, src_file):
        print("failed to unzip source: %s" % src_file)
        return False

    try:
        os.chdir(unzipped_src_dir)
        os.system(
            'cmake '
            '-Wno-dev -DBUILD_SHARED_LIBS=OFF '
            f'-DCMAKE_INSTALL_PREFIX={install_dir!r} -B build .')
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
