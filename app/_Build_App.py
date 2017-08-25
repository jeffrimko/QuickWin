"""Builds the application binaries."""

##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import os

import auxly.filesys as fs
import auxly.shell as sh
import qprompt
import yaml

from _Cleanup import cleanup

##==============================================================#
## SECTION: Global Definitions                                  #
##==============================================================#

# Maintains the build type.
BTYPE = None

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

@qprompt.status("Killing running QuickWin...")
def kill():
    sh.silent("taskkill /f /im quickwin.exe")

def run():
    global BTYPE
    if None == BTYPE:
        qprompt.alert("[WARNING] Build app first!")
        return
    with fs.Cwd(BTYPE):
        kill()
        sh.call("start quickwin.exe")

def build(config, btype="release"):
    global BTYPE
    BTYPE = btype
    # Build application.
    sh.call("uic mainwindow.ui >> ui_mainwindow.h")
    sh.call("qmake")
    if not os.path.exists(BTYPE):
        fs.makedirs(BTYPE)
    status = sh.call("make %s" % (BTYPE))
    try:
        # Copy over DLLs.
        for dll in config['dlls']:
            fs.copy(dll, BTYPE)
        # Copy over assets.
        for ast in config['assets']:
            fs.copy(ast, BTYPE)
        # Remove unnecessary files from build dir.
        for f in os.listdir(BTYPE):
            if any([f.endswith(e) for e in config['build_dir_ext_rm']]):
                fs.delete(os.path.join(BTYPE, f))
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
    qprompt.title("QuickWin Build")
    config = yaml.load(open("build.yaml").read())
    menu = qprompt.Menu()
    menu.add("b", "Build release", build, [config, "release"])
    menu.add("d", "Build debug", build, [config, "debug"])
    menu.add("r", "Run app", run)
    menu.add("k", "Kill app", kill)
    menu.add("c", "Cleanup", cleanup)
    menu.main(loop=True)
