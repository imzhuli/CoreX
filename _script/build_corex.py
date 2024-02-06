#!/usr/bin/env python3

import _check_env as ce
import os
import shutil

if __name__ != "__main__":
    print("not valid entry, name=%s" % (__name__))
    exit

if not ce.check_env():
    exit

cwd = os.getcwd()
build_path = cwd + "/_corex_build"
full_install_path = cwd + "/_corex_installed"
try:
    if os.path.exists(build_path):
        shutil.rmtree(build_path)
    if os.path.exists(full_install_path):
        shutil.rmtree(full_install_path)
    os.mkdir(build_path)
    os.mkdir(full_install_path)
except:
    print("failed to check and remove install target")

os.system(
    'cmake -Wno-dev '
    '-DBUILD_SHARED_LIBS=OFF '
    f'-DCMAKE_INSTALL_PREFIX={full_install_path!r} -B {build_path!r} .')
os.system(f"cmake --build {build_path} -- all")
os.system(f"cmake --build {build_path} -- install")
