import _check_env as ce


def prepare_3rd():
    if not ce.check_env():
        return False
    if not ce.remake_3rd_install_dir():
        return False
    if not ce.remake_3rd_build_dir():
        return False
    return True
