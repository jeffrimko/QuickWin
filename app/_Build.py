"""Builds the application binaries."""

##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import os
import os.path as op

import auxly.filesys as fs
import auxly.shell as sh
import qprompt
import yaml
from ubuild import menu, main

##==============================================================#
## SECTION: Global Definitions                                  #
##==============================================================#

# Maintains the build type.
BTYPE = ("release" if op.exists("release\quickwin.exe") else
        "debug" if op.exists("debug\quickwin.exe") else
        None)

CONFIG = yaml.load(open("build.yaml").read())

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

@menu("d", "Build debug", ['debug'])
@menu("b", "Build release")
def build(btype="release"):
    global BTYPE
    BTYPE = btype
    # Build application.
    sh.call("uic mainwindow.ui >> ui_mainwindow.h")
    sh.call("qmake")
    if not op.exists(BTYPE):
        fs.makedirs(BTYPE)
    status = sh.call("make %s" % (BTYPE))
    try:
        # Copy over DLLs.
        for dll in CONFIG['dlls']:
            fs.copy(dll, BTYPE)
        # Copy over assets.
        for ast in CONFIG['assets']:
            fs.copy(ast, BTYPE)
        # Remove unnecessary files from build dir.
        for f in os.listdir(BTYPE):
            if any([f.endswith(e) for e in CONFIG['build_dir_ext_rm']]):
                fs.delete(op.join(BTYPE, f))
    except:
        qprompt.alert("File copy failed! Try killing app.")
        status = 1
    if status == 0:
        qprompt.alert("Build succeeded.")
    else:
        qprompt.error("Build failed!")

@menu
def clean():
    fs.delete(".", regex=r"\.pyc$")
    fs.delete("release")
    fs.delete("debug")
    fs.delete("ui_mainwindow.h")
    fs.delete("Makefile")
    fs.delete("Makefile.Debug")
    fs.delete("Makefile.Release")

@menu
def run():
    global BTYPE
    if None == BTYPE:
        qprompt.alert("[WARNING] Build app first!")
        return
    with fs.Cwd(BTYPE):
        kill()
        sh.call("start quickwin.exe")

@qprompt.status("Killing running QuickWin...")
@menu
def kill():
    sh.silent("taskkill /f /im quickwin.exe")

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    main(default="b")
