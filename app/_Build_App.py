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

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

def build(config):
    build = "release"
    if not qprompt.ask_yesno("Build release (else debug)?", dft=config['release']):
        build = "debug"
    # Build application.
    call("taskkill /f /im quickwin.exe")
    call("uic mainwindow.ui >> ui_mainwindow.h")
    call("qmake")
    if not os.path.exists(build):
        os.makedir(build)
    call("make %s" % (build))
    # Copy over Qt DLLs.
    for dll in config['qt_dlls']:
        src = os.path.join(config['qt_bin_dir'], dll)
        shutil.copy(src, build)
    # Copy over assets.
    for ast in config['assets']:
        shutil.copy(ast, build)
    # Remove unnecessary files from build dir.
    for f in os.listdir(build):
        if any([f.endswith(e) for e in config['build_dir_ext_rm']]):
            os.remove(os.path.join(build, f))
    # Run app.
    if qprompt.ask_yesno("Run app?", dft=True):
        retdir = os.getcwd()
        os.chdir(build)
        call("start quickwin.exe")
        os.chdir(retdir)

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    config = yaml.load(open("build.yaml").read())
    build(config)
