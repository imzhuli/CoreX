#!/usr/bin/env python3

import _prepared_3rd as p3
import _build_glfw3 as bglfw3
import _build_libcurl as blibcurl
import _build_mbedtls as bmbedtls
import os
import shutil

if __name__ != "__main__":
    print("not valid entry, name=%s" % (__name__))
    exit

if not p3.prepare_3rd():
    exit

if not bglfw3.build():
    exit

if not blibcurl.build():
    exit

if not bmbedtls.build():
    exit

# remove temp dir
cwd = os.getcwd()
dependency_unzip_dir = f"{cwd}/_3rd_build"
shutil.rmtree(dependency_unzip_dir)
