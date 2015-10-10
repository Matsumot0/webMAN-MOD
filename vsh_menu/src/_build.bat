@echo off
del wm_vsh_menu.*
make
del wm_vsh_menu.prx
del wm_vsh_menu.sym
pause
del /q objs\*.d
del /q objs\*.o
rd objs

