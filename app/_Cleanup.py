##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import shutil
import os

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

def rm_if_exists(target, is_file=True):
    if os.path.exists(target):
        if is_file:
            os.remove(target)
        else:
            shutil.rmtree(target)

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    rm_if_exists("release", is_file=False)
    rm_if_exists("debug", is_file=False)
    rm_if_exists("ui_mainwindow.h")
    rm_if_exists("Makefile")
    rm_if_exists("Makefile.Debug")
    rm_if_exists("Makefile.Release")
