::=============================================================::
:: DEVELOPED 2015, REVISED 2017, Jeff Rimko.                   ::
::=============================================================::

::=============================================================::
:: SECTION: Environment Setup                                  ::
::=============================================================::

@set TITLE=%~n0 "%~dp0"
@cd /d %~dp0 && echo off && title %TITLE%

::=============================================================::
:: SECTION: Main Body                                          ::
::=============================================================::

call:ChkDep^
    "G++"^
    "C++ Compiler"^
    "http://www.mingw.org/"^
    "latest"^
    g++ -v
call:ChkDep^
    "Qt"^
    "C++ Application Framework"^
    "http://www.qt.io/"^
    "4.8.7"^
    qmake -v
call:ChkDep^
    "Python"^
    "Python language interpreter."^
    "www.python.org"^
    "latest"^
    python -V
call:ChkDep^
    "PyYAML"^
    "Python YAML library."^
    "http://pyyaml.org/"^
    "latest"^
    python -c "import yaml"
call:ChkDep^
    "Ubuild"^
    "Python build script framework."^
    "github.com/jeffrimko/Ubuild"^
    "latest"^
    python -c "import ubuild"

pause
exit /b 0

::=============================================================::
:: SECTION: Function Definitions                               ::
::=============================================================::

::-------------------------------------------------------------::
:: Checks if a dependency is available.
::
:: **Params**:
::  - 1 - Name of dependency.
::  - 2 - Description of dependency.
::  - 3 - Reference website or where to obtain info.
::  - 4 - Recommended version.
::  - 5+ - Non-blocking command to check if installed; usually version display
::         or help.
::
:: **Attention**:
:: Do not use quotes around the non-blocking command.
:: Quotes may be included in the remaining params if they are needed for the
:: non-blocking call.
::
:: **Preconditions**:
:: The global variable DEP_OK should be set to 1 before the first call to this
:: function.
::
:: **Postconditions**:
:: The global variable DEP_OK will be set to 0 if a dependency check fails.
:: This variable is not set back to 1 by this function, it may be explicitly
:: set outside the function
::
:: **Example**:
:: call::ChkDep^
::     "Utility"^
::     "Does something."^
::     "www.website.com"^
::     "1.2.3"^
::     utility -h
:: call::ChkDep^
::     "Utility"^
::     "Does something."^
::     "www.website.com"^
::     "1.2.3"^
::     utility -c "non-blocking cmd"
::-------------------------------------------------------------::
:ChkDep
echo Checking dependency for %~1...
shift
echo     %~1
shift
echo     Reference: %~1
shift
echo     Recommended version: %~1
shift
echo     --------
set CMD=%1
shift
:chkdep_shift_next
if [%1] neq [] (
    set CMD=%CMD% %1
    shift
    goto:chkdep_shift_next
)
%CMD% > NUL 2>&1
if %ERRORLEVEL% neq 0 (
    echo     NOT FOUND!
    set DEP_OK=0
    goto:eof
)
echo     OK.
goto:eof
