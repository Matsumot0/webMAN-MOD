@echo off

set CYGWIN=C:\cygwin\bin

if not exist %CYGWIN%\bash.exe set CYGWIN=C:\msys\1.0\bin

set CHERE_INVOKING=1

cls

del /q .\webftp_server*.*>nul

del /q objs\cobra\*.d>nul
del /q objs\cobra\*.o>nul
rd objs\cobra>nul

attrib -r objs\*.*

del /q objs\*.d>nul
del /q objs\*.o>nul
rd objs>nul

cls

copy .\flags\flags_cobra.h .\flags.h >nul
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_cobra.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

attrib +r objs\libc.ppu.*
attrib +r objs\printf.ppu.*

del /q objs\*.d>nul
del /q objs\*.o>nul
rd objs>nul

copy .\flags\flags_ps3mapi.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_ps3mapi.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_english.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_english.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_rebug_cobra_multi23.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_rebug_cobra_multi23.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_rebug_cobra_ps3mapi.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_rebug_cobra_ps3mapi.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_rebug_cobra_english.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_rebug_cobra_english.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_noncobra.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_noncobra.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_ccapi.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_ccapi.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_lite.h .\flags.h
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_lite.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\*.d>nul
del /q objs\*.o>nul

copy .\flags\flags_custom.h .\flags.h >nul
%CYGWIN%\bash --login -i -c 'make'
if exist webftp_server.sprx ren webftp_server.sprx webftp_server_full.sprx

del /q webftp_server.elf>nul
del /q webftp_server.sym>nul

del /q objs\cobra\*.d>nul
del /q objs\cobra\*.o>nul
rd objs\cobra>nul

attrib -r objs\*.*

del /q objs\*.d>nul
del /q objs\*.o>nul
rd objs>nul

ren webftp_server_cobra.sprx webftp_server.sprx

