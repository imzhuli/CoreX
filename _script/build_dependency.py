import _prepared_3rd as p3
import _build_glfw3 as bglfw3
import _build_mbedtls as bmbedtls

if __name__ != "__main__":
    print("not valid entry, name=%s" % (__name__))
    exit

if not p3.prepare_3rd():
    exit

if not bglfw3.build():
    exit

if not bmbedtls.build():
    exit
