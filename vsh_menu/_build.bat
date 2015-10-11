@echo off
del wm_vsh_menu.*
make
del wm_vsh_menu.prx
del wm_vsh_menu.sym
del /q objs\*.d
del /q objs\*.o
del /q libc.ppu.d
del /q libc.ppu.o
rd objs

pause
