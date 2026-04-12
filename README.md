## CSSv34 Engine

This project is aimed to improve the leaked Source 2007 code, to make it more stable and to make it compatible with v34 servers.
<br>Warning: Some of the features may fuck up the game<br>

#### Sources:
* [quiver-engine](https://github.com/quiverteam/engine)
* [source-sdk-2006](https://github.com/Source-SDK-Archives/source-sdk-2006-ep1)
* [my cssv34-sdk](https://github.com/rusherr-c/cssv34-sdk)
* [source-2007](https://github.com/uvbs/source-2007)
* [nillerusr's source-engine](https://github.com/nillerusr/source-engine)
* [csgo engine](https://github.com/EpicSentry/HL2-CSGO)

#### Features:
* Recompiled VPC, src from TF2 leak
* Updated headers from CSGO and TF2 leak
* PreInstalled RevEmu 9.85
* VPK support
* Filesystem from TF2 leak (less hardcoded stuff, allows for 'custom' folder, etc)
* Some VGUI stuff ported from TF2 leak.
* VPhysics, havok\ivp code included.
* CVAR culling disabled.
* DirectX SDK from Summer 2004 (doesn't require d3dx9_**.dll)
* vaudio_minimp3 from TF2 leak
* Deleted bink video
* Rewrited serverbrowser (no longer depends on steam)

#### Currently known problems:
* sv_pure is broken
* FCVAR_NOTIFY is broken
* some netmessages is fully incompatible with v34 version
* some network string tables is also incompatible with v34 version
* game constantly lags every second, idk why

## Troubleshooting compiling
Having problems building the project? Make sure you have the following:<br>
- Windows 10 SDK: https://developer.microsoft.com/en-us/windows/downloads<br>
- MFC for v145 build tools: Available under the "Individual Components" section of the Visual Studio Installer.<br>
VS 2026 should work with no additional changes necessary.<br>

