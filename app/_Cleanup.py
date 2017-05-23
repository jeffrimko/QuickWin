##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import auxly.filesys as fs

##==============================================================#
## SECTION: Function Definitions                                #
##==============================================================#

def cleanup():
    fs.delete("release")
    fs.delete(".", regex=r"\.pyc$")
    fs.delete("debug")
    fs.delete("ui_mainwindow.h")
    fs.delete("Makefile")
    fs.delete("Makefile.Debug")
    fs.delete("Makefile.Release")

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    cleanup()
