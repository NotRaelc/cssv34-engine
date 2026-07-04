@echo off

set cmd_line=-game cstrike -dev -debug -sw -condebug %*

start "" hl2.exe %cmd_line%
echo hl2.exe %cmd_line%

timeout 5