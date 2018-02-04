@echo off
SET msvcdir_old="C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"
SET msvcdir="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\"
if not defined DevEnvDir call %msvcdir%vcvars64.bat >nul

SET MAIN_FILE=src\vmf_main.c
SET META_OUT=src\vmf_generated.h
SET BIN_DIR=bin
SET BASE_NAME=VMFSketch
SET EXEOUT=%BIN_DIR%\%BASE_NAME%.exe
SET PDBOUT=%BIN_DIR%\%BASE_NAME%.pdb

SET LIBRARY_PATH=msvc_libs\lib
SET INCLUDE_PATH=msvc_libs\include
SET SHARED_PATH=msvc_libs\shared

SET DISABLED_WARNINGS=/wd4477 ^
	/wd4244 ^
	/wd4267 ^
	/wd4334 ^
	/wd4305

SET DEBUG_LIBS=
SET RELEASE_LIBS=

SET LIBS=kernel32.lib ^
	user32.lib ^
	gdi32.lib ^
	SDL2.lib ^
	SDL2main.lib ^
	Comdlg32.lib ^
	opengl32.lib

SET ASSET_ARCHIVE=%BIN_DIR%\assets.zip
SET zip=utilities\7z.exe
SET metaprogram=utilities\metaprogram.exe
SET ctime=utilities\ctime.exe

taskkill /IM %BASE_NAME%.exe >NUL 2>&1

%ctime% -begin vmfsketch.ctm

%metaprogram% -p -t -s %MAIN_FILE% > %META_OUT%

IF EXIST %BIN_DIR% GOTO BUILDSTART
mkdir %BIN_DIR%
:BUILDSTART

REM cl explanation:
REM nologo: disables "Microsoft Optimizing Compiler..."
REM TC: globally use C language -- /TP for global C++
REM W3: enable warning level 3
REM Gd: use cdecl calling convention. Gr is fastcall, Gz is stdcall, Gv is vectorcall 
REM GS-: disable buffer security checks
REM Gs****: set maximum size of local variables before there's a stack probe
REM fp:fast: use fast floating point
REM EHsc: Only catch C++ exceptions, extern "C" will never throw a C++ execption
REM MTd: statically link against CRT (debug), MD is dynamic link. LD creates a dll
REM wd****: warning disable
REM D****: define ****. Use /Dname#value for numeric values
REM Fe: rename executable (File executable)
REM Fd: rename pdb (File database)
REM link explanation:
REM ****.lib: library to link against
REM SUBSYSTEM:****: use WINDOWS or CONSOLE
REM nologo: same as above
REM INCREMENTAL:NO: don't do incremental linking or generate ikl files

REM Not included here, but might be useful
REM F *****: sets the size of the stack; 1mb = 1048576, 4mb = 4194308
REM /link
REM /NODEFAULTLIB: cast away your shackles of mortality and ascend the CRT, becoming one with the system
REM (note, remove /M(T/D)(d) to make /NODEFAULTLIB real)

if "%~1"=="publish" goto RELEASE_BUILD

if "%~1"=="debug" goto DEBUG_BUILD
if "%~1"=="release" goto RELEASE_BUILD
if "%~1"=="run" goto DEBUG_BUILD

:DEBUG_BUILD
cl ^
	/nologo ^
	/I %INCLUDE_PATH% ^
	/TC ^
	/Zi ^
	/W3 ^
	/Gd ^
	/fp:fast ^
	/EHsc ^
	/MTd ^
	%DISABLED_WARNINGS% ^
	%MAIN_FILE% ^
	/DWB_DEBUG ^
	/DWB_WINDOWS ^
	/Fe%EXEOUT% ^
	/Fd%PDBOUT% ^
	/link ^
	app.res ^
	/LIBPATH:%LIBRARY_PATH% ^
	%LIBS% ^
	%DEBUG_LIBS% ^
	/SUBSYSTEM:CONSOLE ^
	/NOLOGO ^
	/INCREMENTAL:NO
GOTO DONE


:RELEASE_BUILD
cl ^
	/nologo ^
	/Zo ^
	/I %INCLUDE_PATH% ^
	/TC ^
	/W3 ^
	/Gd ^
	/fp:fast ^
	/EHsc ^
	/MT ^
	/O2 ^
	%DISABLED_WARNINGS% ^
	%MAIN_FILE% ^
	/DWB_RELEASE ^
	/DWB_WINDOWS ^
	/Fe%EXEOUT% ^
	/Fd%PDBOUT% ^
	/link ^
	app.res ^
	/LIBPATH:%LIBRARY_PATH% ^
	%LIBS% ^
	%RELEASE_LIBS% ^
	/SUBSYSTEM:WINDOWS ^
	/NOLOGO ^
	/INCREMENTAL:NO

:DONE
del *.obj

:DEPENDENCIES
copy %SHARED_PATH%\*.dll %BIN_DIR% 1>NUL 2>&1
del %ASSET_ARCHIVE% 1>NUL 2>&1
del bin\vmfsketch.dat 1>NUL 2>&1
%zip% a %ASSET_ARCHIVE% src\shaders\*.glsl 1>NUL
%zip% a %ASSET_ARCHIVE% assets\*.png 1>NUL
%zip% a %ASSET_ARCHIVE% assets\*.wav 1>NUL
%zip% a %ASSET_ARCHIVE% assets\*.ogg 1>NUL
%zip% a %ASSET_ARCHIVE% assets\*.flac 1>NUL
%zip% a %ASSET_ARCHIVE% assets\*.ttf 1>NUL
del %BIN_DIR%\*.tmp* 1>NUL 2>&1
ren %ASSET_ARCHIVE% vmfsketch.dat

if "%~1"=="publish" goto PUBLISH

if "%~1"=="run" goto RUN
if "%~2"=="run" goto RUN
GOTO END

:RUN
start %EXEOUT%
GOTO END

:PUBLISH
SET PUBLISH_ARCHIVE=..\%BASE_NAME%.zip
pushd %BIN_DIR%
..\%zip% a %PUBLISH_ARCHIVE% *.exe 1>NUL 
..\%zip% a %PUBLISH_ARCHIVE% *.dll 1>NUL 
..\%zip% a %PUBLISH_ARCHIVE% *.zip 1>NUL 
popd

GOTO END

:END
%ctime% -end vmfsketch.ctm

