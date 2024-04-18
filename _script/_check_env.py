#!/usr/bin/env python3

import sys
import os
import shutil
import _cmake_util as cu

MIN_PY_VERSION_MAJOR = 3
MIN_PY_VERSION_MINOR = 7

def check_version():
    py_version_major = sys.version_info[0]
    py_version_minor = sys.version_info[1]
    if py_version_major < MIN_PY_VERSION_MAJOR:
        print("invalid python major version: at least %u" %
              MIN_PY_VERSION_MAJOR)
        return False
    if py_version_major == MIN_PY_VERSION_MAJOR and py_version_minor < MIN_PY_VERSION_MINOR:
        print("invalid python ninor version: at least %u" %
              MIN_PY_VERSION_MINOR)
        return False
    return True


def check_tag_file():
    cwd = os. getcwd()
    tag_filename = cwd + "/CoreXX.tag"
    if not os.path.isfile(tag_filename):
        print("tag file not found, please check the working directory")
        return False
    return True

def check_env():
    valid = check_version() and check_tag_file()
    if not valid:
        print("Failed to pass all env requirements")
        return False
    print("Basic dependency checked.")
    return True

def remake_dir(dir):
    try:
        shutil.rmtree(dir, onerror=cu.remove_readonly)
    except Exception as e:
        print(f"RM: Exception: {e}")
        pass
    try:
        os.mkdir(dir)
    except Exception as e:
        print(f"MK: Exception: {e}")
        return False
    return True


def remake_3rd_build_dir():
    cwd = os. getcwd()
    build_dir = cwd + "/_3rd_build"
    if not remake_dir(build_dir):
        print("failed to remake 3rd_build dir")
        return False
    print("3rd_build dir prepared: " + build_dir)
    return True


def remake_3rd_install_dir():
    cwd = os. getcwd()
    install_dir = cwd + "/_3rd_installed"
    if not remake_dir(install_dir):
        print("failed to remake 3rd_installed dir")
        return False
    print("3rd_installed dir prepared: " + install_dir)
    return True
