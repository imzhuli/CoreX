#!/usr/bin/env python3

import _prepared_3rd as p3
import _build_freetype as bfreetype
import _build_glfw3 as bglfw3
import _build_glm as bglm
import _build_libcurl as blibcurl
import _build_libwebsockets as bws
import _build_mbedtls as bmbedtls
import _build_rapidjson as brapidjson
import _build_zlib as bzlib
import getopt
import os
import shutil
import sys

if __name__ != "__main__":
    print("not valid entry, name=%s" % (__name__))
    exit

server_side_lib_only = "OFF"
try:
    argv = sys.argv[1:]
    opts, args = getopt.getopt(argv, "sr")
except getopt.GetoptError:
    sys.exit(2)
for opt, arg in opts:
    if opt == "-s":
        server_side_lib_only = "ON"
        print("server-side lib only")
    if opt == '-r':
        os.environ["CMAKE_BUILD_TYPE"] = "Release"
        print("set CMAKE_BUILD_TYPE to Release")
    pass

if not p3.prepare_3rd():
    exit

if not bfreetype.build():
    exit

if server_side_lib_only == "OFF":
    if not bglfw3.build():
        exit

if not bglm.build() or not bglm.post_build():
    exit

if not bmbedtls.build():
    exit

if not brapidjson.build():
    exit

if not bzlib.build():
    exit

# build curl after mbedtls
if not blibcurl.build():
    exit

if not bws.build():
    exit

# remove temp dir
# cwd = os.getcwd()
# dependency_unzip_dir = f"{cwd}/_3rd_build"
# shutil.rmtree(dependency_unzip_dir)
