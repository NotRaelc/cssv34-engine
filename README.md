# CSSv34 Engine

Source Engine 2007 but with CSSv34 server-connection support.
<br>Warning: this project is WIP, some of the features may not work properly!

Sources:
* [quiver-engine](https://github.com/quiverteam/engine)
* [source-sdk-2006](https://github.com/Source-SDK-Archives/source-sdk-2006-ep1)
* [my cssv34-sdk](https://github.com/rusherr-c/cssv34-sdk)
* [source-2007](https://github.com/uvbs/source-2007)
* [nillerusr's source-engine](https://github.com/nillerusr/source-engine)
* [csgo engine](https://github.com/EpicSentry/HL2-CSGO)

Features:<br>
* Recompiled VPC, src from TF2 leak
* Updated headers from CSGO and TF2 leak
* Steam_API stub included
* VPK support
* Removed Scaleform.
* Filesystem from TF2 leak (less hardcoded stuff, allows for 'custom' folder, etc)
* Some VGUI stuff ported from TF2 leak.
* VPhysics, havok\ivp code included.
* CVAR culling disabled.
* DirectX SDK from Summer 2004 (doesn't require d3dx9_**.dll)
* vaudio_minimp3 from TF2 leak
* Deleted valve_avi & bink
* Added video_services

# Troubleshooting compiling
Having problems building the project? Make sure you have the following:<br>
- Windows 10 SDK: https://developer.microsoft.com/en-us/windows/downloads<br>
- MFC for v143 or v145 build tools: Available under the "Individual Components" section of the Visual Studio Installer.<br>
VS 2022-2026 should work with no additional changes necessary.<br>

