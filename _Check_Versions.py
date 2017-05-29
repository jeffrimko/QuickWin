##==============================================================#
## SECTION: Imports                                             #
##==============================================================#

import verace

##==============================================================#
## SECTION: Global Definitions                                  #
##==============================================================#

VERCHK = verace.VerChecker("Qprompt", __file__)
VERCHK.include(r"app\mainwindow.cpp", match="QString gVerStr", splits=[('"',1)])
VERCHK.include(r"CHANGELOG.adoc", match="quickwin-", splits=[("-",1),(" ",0)], updatable=False)

##==============================================================#
## SECTION: Main Body                                           #
##==============================================================#

if __name__ == '__main__':
    VERCHK.prompt()
