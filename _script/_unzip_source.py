#!/usr/bin/env python3

import tarfile
import zipfile
import os

def get_longest_extensions(filename):
    parts = filename.split('.')
    part_num = len(parts)
    if (part_num <= 1) :
        return ""
    extensions = '.'.join(parts[-(part_num-1):])
    return extensions

def auto_extract(dst_dir: str, src_file: str):
    ext = get_longest_extensions(src_file)
    if ext == "zip":
        return unzip_source(dst_dir, src_file)
    if ext == "tar.gz":
        return extract_tarfile(dst_dir, src_file)
    if ext == "tar.xz":
        return extract_tarfile(dst_dir, src_file)


def extract_tarfile(dst_dir: str, src_file: str):
    try:
        with tarfile.open(src_file) as file:
            file.extractall(dst_dir)
    except Exception as e:
        print(f"extract_tarfile error: {e}")
        return False
    return True

def unzip_source(dst_dir: str, src_file: str):
    if not os.path.isfile(src_file):
        print("file not found: %s" % src_file)
        return False
    try:
        with zipfile.ZipFile(src_file, 'r') as zip_ref:
            zip_ref.extractall(dst_dir)
    except Exception as e:
        print("failed to unzip source file, error=%s" % e)
        return False
    return True

