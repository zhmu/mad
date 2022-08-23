@echo off
del ..\source\data.mad >NUL
echo Building MAD archive...
..\utils\mfile.exe -n ..\source\data.mad < MFILE.IN
echo.
echo Done
