@echo off
echo This utility creates junctions to hl2 and cstrike folders
echo All you need is original CSS v34 content
echo.
set /p "in1=Enter path to your <CSSv34>\hl2 folder: "
set /p "in2=Enter path to your <CSSv34>\cstrike folder: "

mklink /j hl2 %in1% >nul
echo Created junction hl2 --^> %in1%
mklink /j cstrike_content %in2% >nul
echo Created junction cstrike_content --^> %in2%
echo.
echo ...Press any key to exit...
pause >nul
