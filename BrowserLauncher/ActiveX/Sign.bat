@echo off
set CAB=WarIncLaunch_1_0_0_9.cab
signtool.exe sign /a /t http://timestamp.verisign.com/scripts/timestamp.dll /v release/WarIncLaunch.dll
makecab /f WarIncLaunch.ddf
signtool.exe sign /a /t http://timestamp.verisign.com/scripts/timestamp.dll /d "War Inc. Battle Zone Launcher" /v %CAB%
del setup.rpt
del setup.inf
