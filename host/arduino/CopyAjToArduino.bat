SETLOCAL

SET SRC=c:\cygwin\home\%USERNAME%\repos\alljoyn\ajtcl
SET DST=c:\arduino\arduino-1.5.2\hardware\arduino\sam\libraries\AllJoyn

for %%I in (%DST%\*.cpp) do del %%I

xcopy /Y /D /S /I %SRC%\inc\* %DST%
xcopy /Y /D /S /I %SRC%\src\* %DST%
xcopy /Y /D /S /I %SRC%\host\arduino\* %DST%

for %%I in (%DST%\*.c) do ren %%I %%~nI.cpp
