import zipfile
import os


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
