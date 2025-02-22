#https://docs.platformio.org/en/latest/scripting/actions.html

Import("env")
import shutil

def pre_filesystem_action(source, target, env):
    f = open("data/version", "w")
    f.write(env.GetProjectOption("filesystem_version").replace(",", ".") + "\n")

def post_firmware_action(source, target, env):
    filename = "firmware_v" + env.GetProjectOption("firmware_version").replace(",", ".") + ".bin"
    program_path = target[0].get_abspath().replace(".elf", ".bin")
    shutil.copy(program_path, "\\\\aoostar\\aoostar\\apache\\website\\files\\" + filename)

def post_filesystem_action(source, target, env):
    filename = "spiffs_v" + env.GetProjectOption("filesystem_version").replace(",", ".") + ".bin"
    program_path = target[0].get_abspath().replace(".elf", ".bin")
    shutil.copy(program_path, "\\\\aoostar\\aoostar\\apache\\website\\files\\" + filename)


env.AddPreAction("$BUILD_DIR/spiffs.bin", pre_filesystem_action)

env.AddPostAction("$BUILD_DIR/firmware.bin", post_firmware_action)
env.AddPostAction("$BUILD_DIR/spiffs.bin", post_filesystem_action)