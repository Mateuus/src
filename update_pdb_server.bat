For /f "tokens=2-4 delims=/ " %%a in ('date /t') do (set date=%%a-%%b-%%c)
"C:\Program Files\Debugging Tools for Windows (x64)\symstore.exe" add /r /f J:\WarOnline\bin\*.* /s J:\symbol_cache /t "WarInc" /v "ver1.0" /c "%date% build"
"C:\Program Files\Debugging Tools for Windows (x64)\symstore.exe" add /r /f J:\WarOnline\server\bin\*.* /s J:\symbol_cache /t "WarIncServer" /v "ver1.0" /c "%date% build"

pause