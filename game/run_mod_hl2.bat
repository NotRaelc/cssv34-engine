@echo off

set cmd_line=-console -sw -noborder -game mod_hl2 -condebug

start "" hl2.exe %cmd_line%
echo hl2.exe %cmd_line%

timeout 5