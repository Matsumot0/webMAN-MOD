@echo off
del wm_vsh_menu.*
make
del wm_vsh_menu.prx
del wm_vsh_menu.sym
del /q objs\*.d
del /q objs\*.o
del /q *.d
del /q *.o
rd objs

