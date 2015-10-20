@echo off
set CYGWIN=C:\cygwin\bin

if not exist %CYGWIN%\bash.exe set CYGWIN=C:\msys\1.0\bin
set CHERE_INVOKING=1

if exist wm_vsh_menu.sprx del /q wm_vsh_menu.sprx>nul
if exist wm_vsh_menu.prx  del /q wm_vsh_menu.prx>nul
if exist wm_vsh_menu.sym  del /q wm_vsh_menu.sym>nul

md data>nul
if exist data\keys del /q data\*>nul
copy ..\data\* data>nul

%CYGWIN%\bash --login -i -c 'make;exit'

del /q data\*>nul
rd data>nul

if exist wm_vsh_menu.prx  del /q wm_vsh_menu.prx>nul
if exist wm_vsh_menu.sym  del /q wm_vsh_menu.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul
del /q *.d>nul
del /q *.o>nul
rd objs

pause
