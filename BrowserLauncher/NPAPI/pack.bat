@echo off
cd xpi
set XPI=npwiu_1_0_0_3.xpi
:start /wait "C:\Program Files\WinRAR\winrar" "a -r -x.svn -xplatform\.svn -xplatform\WINNT_x86-msvc\.svn -xplatform\WINNT_x86-msvc\plugins\.svn ..\%XPI%.zip
pkzipc -add -dir -excl=.svn ..\%XPI%.zip
cd ..
del %XPI% 2> nul
ren %XPI%.zip %XPI%
