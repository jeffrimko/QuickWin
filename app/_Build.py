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

CONFIG = yaml.load(open("build.yaml").read(), Loader=yaml.FullLoader)

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
    qprompt.hrule()
    if status == 0:
        qprompt.alert("Build succeeded.")
    else:
        qprompt.error("Build failed!")
    qprompt.hrule()

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
        if is_running(verbose=False):
            kill()
        sh.call("start quickwin.exe")
        if not is_running():
            qprompt.alert("Could not start QuickWin!")

@menu
def kill():
    qprompt.alert("Killing QuickWin...")
    sh.silent("taskkill /f /im quickwin.exe")
    if is_running():
        qprompt.warn("Could not kill QuickWin!")

@menu
def is_running(verbose=True):
    out = sh.strout('tasklist /nh /fi "imagename eq quickwin.exe"')
    running = not out.startswith("INFO: No tasks are running")
    if verbose:
        if running:
            qprompt.alert("QuickWin is running.")
        else:
            qprompt.alert("QuickWin is not running.")
    return running

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    # print(is_running())
    main(default="b")
