#!/usr/bin/env python3

import _check_env as ce
import getopt
import os
import shutil
import sys
import xsetup

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

server_side_lib_only = "OFF"
j_threads = ""
try:
    argv = sys.argv[1:]
    opts, args = getopt.getopt(argv, "srj:")
except getopt.GetoptError:
    sys.exit(2)
for opt, arg in opts:
    if opt == "-s":
        server_side_lib_only = "ON"
        print("server-side lib only")
    if opt == "-j":
        j_threads = " -j%s " % arg
    if opt == '-r':
        xsetup.Release()
    pass

xsetup.Output()
prepare = \
    'cmake ' \
    f'{xsetup.cmake_build_type} ' \
    '-Wno-dev ' \
    '-DBUILD_SHARED_LIBS=OFF ' \
    f'-DSERVER_SIDE_LIB_ONLY={server_side_lib_only} ' \
    f'-DCMAKE_INSTALL_PREFIX="{full_install_path}" -B "{build_path}" .'
os.system(prepare)
os.system(f'cmake --build "{build_path}" {xsetup.cmake_build_config} -- {j_threads}')
os.system(f'cmake --install "{build_path}" {xsetup.cmake_build_config}')
