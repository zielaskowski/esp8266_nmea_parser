import shutil
from os import write

Import("env")


def assembly_step(source, target, env):
    f = source[0]
    f_txt = source[0].get_contents()
    with open("./test/web_server.i", "w", encoding="UTF8") as f:
        f.write(str(f_txt))


env.AddPreAction("$BUILD_DIR/src/web_server.o", assembly_step)
