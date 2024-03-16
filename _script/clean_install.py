import _check_env as ce
import os

if __name__ != "__main__":
    print("not valid entry, name=%s" % (__name__))
    exit

if not ce.check_env():
    print("invalid env check result")
    exit

cwd = os.getcwd()
build_path = cwd + "/_corex_build"
full_install_path = cwd + "/_corex_installed"

ce.remake_dir(full_install_path)
