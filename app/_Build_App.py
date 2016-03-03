"""Builds the application binaries."""

##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import os
import functools
import subprocess
import shutil

import yaml

import qprompt

##==============================================================#
## SECTION: Global Definitions                                  #
##==============================================================#

# Function alias.
call = functools.partial(subprocess.call, shell=True)

# Maintains the build type.
BUILD = None

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

def kill():
    call("taskkill /f /im quickwin.exe")

def run():
    global BUILD
    if None == BUILD:
        qprompt.alert("[WARNING] Build app first!")
        return
    retdir = os.getcwd()
    os.chdir(BUILD)
    kill()
    call("start quickwin.exe")
    os.chdir(retdir)

def build(config):
    global BUILD
    BUILD = "release"
    if not qprompt.ask_yesno("Build release (else debug)?", dft=config['release']):
        BUILD = "debug"
    # Build application.
    call("uic mainwindow.ui >> ui_mainwindow.h")
    call("qmake")
    if not os.path.exists(BUILD):
        os.makedir(BUILD)
    status = call("make %s" % (BUILD))
    try:
        # Copy over Qt DLLs.
        for dll in config['qt_dlls']:
            src = os.path.join(config['qt_bin_dir'], dll)
            shutil.copy(src, BUILD)
        # Copy over assets.
        for ast in config['assets']:
            shutil.copy(ast, BUILD)
        # Remove unnecessary files from build dir.
        for f in os.listdir(BUILD):
            if any([f.endswith(e) for e in config['build_dir_ext_rm']]):
                os.remove(os.path.join(BUILD, f))
    except:
        qprompt.alert("File copy failed! Try killing app.")
        status = 1
    # Run app.
    if status == 0:
        if qprompt.ask_yesno("Run app?", dft=True):
            run()
    else:
        qprompt.alert("ERROR: Build failed!")

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    config = yaml.load(open("build.yaml").read())
    menu = qprompt.Menu()
    menu.add("b", "build app", build, [config])
    menu.add("r", "run app", run)
    menu.add("k", "kill app", kill)
    menu.add("q", "quit")
    while "q" != menu.show():
        pass
