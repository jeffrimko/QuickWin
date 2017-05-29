"""Builds the application binaries."""

##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import os
import auxly.filesys as fs
import auxly.shell as sh

import yaml

import qprompt

from _Cleanup import cleanup

##==============================================================#
## SECTION: Global Definitions                                  #
##==============================================================#

# Maintains the build type.
BUILD = None

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

def kill():
    sh.call("taskkill /f /im quickwin.exe")

def run():
    global BUILD
    if None == BUILD:
        qprompt.alert("[WARNING] Build app first!")
        return
    retdir = os.getcwd()
    os.chdir(BUILD)
    kill()
    sh.call("start quickwin.exe")
    os.chdir(retdir)

def build(config, btype="release"):
    global BUILD
    BUILD = btype
    # Build application.
    sh.call("uic mainwindow.ui >> ui_mainwindow.h")
    sh.call("qmake")
    if not os.path.exists(BUILD):
        os.makedir(BUILD)
    status = sh.call("make %s" % (BUILD))
    try:
        # Copy over DLLs.
        for dll in config['dlls']:
            fs.copy(dll, BUILD)
        # Copy over assets.
        for ast in config['assets']:
            fs.copy(ast, BUILD)
        # Remove unnecessary files from build dir.
        for f in os.listdir(BUILD):
            if any([f.endswith(e) for e in config['build_dir_ext_rm']]):
                fs.delete(os.path.join(BUILD, f))
    except:
        qprompt.alert("File copy failed! Try killing app.")
        status = 1
    if status == 0:
        qprompt.alert("Build succeeded.")
    else:
        qprompt.error("Build failed!")

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    config = yaml.load(open("build.yaml").read())
    menu = qprompt.Menu()
    menu.add("b", "Build release", build, [config, "release"])
    menu.add("d", "Build debug", build, [config, "debug"])
    menu.add("r", "Run app", run)
    menu.add("k", "Kill app", kill)
    menu.add("c", "Cleanup", cleanup)
    menu.main(loop=True)
