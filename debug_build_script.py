#https://docs.platformio.org/en/latest/scripting/actions.html

Import("env")

def pre_filesystem_action(source, target, env):
    f = open("data/version", "w")
    f.write(env.GetProjectOption("filesystem_version").replace(",", ".") + "\n")


env.AddPreAction("$BUILD_DIR/spiffs.bin", pre_filesystem_action)