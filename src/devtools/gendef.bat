@echo off
echo LIBRARY %1.dll > %1.def
echo EXPORTS >> %1.def
for /f "skip=19 tokens=4" %%A in ('dumpbin /exports %1.dll') do echo %%A >> %1.def
