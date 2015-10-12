@echo off
if exist wm_vsh_menu.sprx del /q wm_vsh_menu.sprx>nul
if exist wm_vsh_menu.prx  del /q wm_vsh_menu.prx>nul
if exist wm_vsh_menu.sym  del /q wm_vsh_menu.sym>nul

md data>nul
if exist data\keys del /q data\*>nul
copy ..\data\* data>nul

make

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