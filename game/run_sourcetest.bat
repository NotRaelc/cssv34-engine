@echo off

set cmd_line=-game sourcetest -dev -sw %*

start "" hl2.exe %cmd_line%
echo hl2.exe %cmd_line%

timeout 5