REM for VS2005
REM cygwin1.7‚Å‚Ìƒrƒ‹ƒh•û–@‚ª‚í‚©‚ç‚È‚¢‚½‚ß

REM call "C:\Program Files\Microsoft Visual Studio 8\VC\vcvarsall.bat" x86
REM call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat" x86
call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86

@echo on

REM Python 3.3
REM SET PYTHON_ROOT=C:\Python33x86

REM Python 3.5
REM SET PYTHON_ROOT=%USERPROFILE%\AppData\Local\Programs\Python\Python35-32
REM SET PYTHON_LIB_FILENAME=python35.lib

REM Python 3.7
SET PYTHON_ROOT=%USERPROFILE%\AppData\Local\Programs\Python\Python37-32
SET PYTHON_LIB_FILENAME=python37.lib

cl /D "WIN32" /D "NDEBUG" /D "_ATL_MIN_CRT" /D "_UNICODE" /D "UNICODE" /FD /EHsc /MT /GR- /c /TP /I%PYTHON_ROOT%\include spi.cpp /Fospi.obj

link /INCREMENTAL:NO /NOLOGO /LIBPATH:%PYTHON_ROOT%\libs /MACHINE:X86 /DLL /OUT:spi.pyd %PYTHON_LIB_FILENAME% spi.obj

del *.lib
del *.exp
del *.idb

pause
