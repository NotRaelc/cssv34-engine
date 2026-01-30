#define _CRT_SECURE_NO_WARNINGS
#define STEAM_API_EXPORTS
#define NULL 0
#include "steamCommon.h"
#include "steam\steam_api.h"
#include "revEmu\RevEmu2013.h" //// https://github.com/kohtep/MultiEmulator ////

#define NOTE_UNUSED(x) (void)(x)
S_API void *g_pSteamClientGameServer;
void *g_pSteamClientGameServer = NULL;

class CSteamUser : public ISteamUser
{
public:
	CSteamUser() : SteamID(0ull) {};
	~CSteamUser() {};
	virtual HSteamUser GetHSteamUser() { return 1; }
	virtual void LogOn(CSteamID steamID) {}
	virtual void LogOff() {}
	virtual bool BLoggedOn() { return 1; }
	virtual ELogonState GetLogonState() { return k_ELogonStateLoggedOn; }
	virtual bool BConnected() { return 1; }
	virtual CSteamID GetSteamID(){ return SteamID; }
	virtual bool IsVACBanned(EVACBan eVACBan){ return 0; }
	virtual bool RequireShowVACBannedMessage(EVACBan eVACBan) { return 0; }
	virtual void AcknowledgeVACBanning(EVACBan eVACBan) {}
	virtual int NClientGameIDAdd(int nGameID) { return 0; }
	virtual void RemoveClientGame(int nClientGameID) {}
	virtual void SetClientGameServer(int nClientGameID, uint32 unIPServer, uint16 usPortServer) {}
	virtual void SetSteam2Ticket(uint8* pubTicket, int cubTicket) {}
	virtual void AddServerNetAddress(uint32 unIP, uint16 unPort) {}
	virtual bool SetEmail(const char* pchEmail) { return 0; }
	virtual int Obsolete_GetSteamGameConnectToken(void* pBlob, int cbMaxBlob) { return 0; }
	virtual bool SetRegistryString(ERegistrySubTree eRegistrySubTree, const char* pchKey, const char* pchValue) { return 0; }
	virtual bool GetRegistryString(ERegistrySubTree eRegistrySubTree, const char* pchKey, char* pchValue, int cbValue) { return 0; }
	virtual bool SetRegistryInt(ERegistrySubTree eRegistrySubTree, const char* pchKey, int iValue) { return 0; }
	virtual bool GetRegistryInt(ERegistrySubTree eRegistrySubTree, const char* pchKey, int* piValue) { return 0; }
	virtual int InitiateGameConnection(void* pBlob, int cbMaxBlob, CSteamID steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure);
	virtual void TerminateGameConnection(uint32 unIPServer, uint16 usPortServer) {}
	virtual void SetSelfAsPrimaryChatDestination() {}
	virtual bool IsPrimaryChatDestination() { return 1; }

private:
	CSteamID	SteamID;
};

// ---
// RevSpoofer by kohtep: https://github.com/kohtep/MultiEmulator
// ---
int CSteamUser::InitiateGameConnection(void* pBlob, int cbMaxBlob, CSteamID steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure) {
	if (steamID.GetAccountID() == 0) return 0; // steam id is invalid

	char szhwid[64];

	generateRandomHWID(szhwid);

	int nSteamID = (rand() % (INT_MAX - 26954 + 1)) + 26954; // https://hlmod.net/threads/revemu-2013-reshenie-protiv-podmeny-stimov.59390/post-545419

	if (!RevSpoofer::Spoof(szhwid, nSteamID))
		return 0;

	auto pTicket = (int*)pBlob;
	auto pbTicket = (unsigned char*)pBlob;

	auto revHash = RevSpoofer::Hash(szhwid);

	pTicket[0] = 'S';                      // +0
	pTicket[1] = revHash;                  // +4
	pTicket[2] = 'rev';                    // +8
	pTicket[3] = 0;                        // +12
	pTicket[4] = revHash * 1;             // +16
	pTicket[5] = 0x01100001;               // +20

	static const char c_szAESKeyRand[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";

	char szAESHashRand[32];
	auto AESRand = CRijndael();
	AESRand.MakeKey(c_szAESKeyRand, CRijndael::sm_chain0, 32, 32);
	AESRand.EncryptBlock(szhwid, szAESHashRand);
	memcpy(&pbTicket[40], szAESHashRand, 32);

	static const char c_szAESKeyRev[] = "_YOU_SERIOUSLY_NEED_TO_GET_LAID_sJ_r$WVsH%zRq&v$fl3jCY7SK3Em3s%f";
	char AESHashRev[32];
	auto AESRev = CRijndael();
	AESRev.MakeKey(c_szAESKeyRev, CRijndael::sm_chain0, 32, 32);
	AESRev.EncryptBlock(c_szAESKeyRand, AESHashRev);
	memcpy(&pbTicket[72], AESHashRev, 32);

	char szSHAHash[32];
	auto sha = CSHA(CSHA::SHA256);
	sha.AddData(szhwid, 32);
	sha.FinalDigest(szSHAHash);
	memcpy(&pbTicket[104], szSHAHash, 32);

	return 178;
	// MUST ALSO TRY:
	// return GenerateRevEmu2013(pBlob, nSteamID);
	// return GenerateSC2009(pBlob, nSteamID);
}

//steam_api.h
S_API bool SteamAPI_Init() {
	return true;
}

S_API bool SteamAPI_InitSafe() {
	return true;
}

S_API void SteamAPI_Shutdown() {

}

S_API bool SteamAPI_RestartAppIfNecessary() {
	return false;
}

S_API void SteamAPI_ReleaseCurrentThreadMemory() {

}

S_API void SteamAPI_WriteMiniDump() {

}

S_API void SteamAPI_SetMiniDumpComment() {

}

S_API void SteamAPI_RunCallbacks() {
}

S_API void SteamAPI_RegisterCallback( class CCallbackBase* pCallback, int iCallback ) {
	NOTE_UNUSED(pCallback); NOTE_UNUSED(iCallback);
}

S_API void SteamAPI_UnregisterCallback( class CCallbackBase* pCallback ) {
	NOTE_UNUSED(pCallback);
}

S_API void SteamAPI_RegisterCallResult() {

}

S_API void SteamAPI_UnregisterCallResult() {

}

S_API bool SteamAPI_IsSteamRunning() {
	return false;
}

S_API void Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks ) {
	NOTE_UNUSED(hSteamPipe); NOTE_UNUSED(bGameServerCallbacks);
}

S_API void Steam_RegisterInterfaceFuncs(void* hModule) {
	NOTE_UNUSED(hModule);
}

S_API int Steam_GetHSteamUserCurrent() {
	return 0;
}

S_API const char *SteamAPI_GetSteamInstallPath() {
	return NULL;
}

S_API int SteamAPI_GetHSteamPipe() {
	return 0;
}

S_API void SteamAPI_SetTryCatchCallbacks() {

}

S_API void SteamAPI_SetBreakpadAppID() {

}

S_API void SteamAPI_UseBreakpadCrashHandler() {

}

S_API int GetHSteamPipe() {
	return 0;
}

S_API int GetHSteamUser() {
	return 0;
}

S_API int SteamAPI_GetHSteamUser() {
	return 0;
}

S_API void *SteamInternal_ContextInit() {
	return NULL;
}

S_API void *SteamInternal_CreateInterface() {
	return NULL;
}

S_API void *SteamApps() {
	return NULL;
}

S_API ISteamClient *SteamClient() {
	return NULL;
}

S_API ISteamFriends *SteamFriends() {
	return NULL;
}

S_API void *SteamHTTP() {
	return NULL;
}

S_API void *SteamMatchmaking() {
	return NULL;
}

S_API void *SteamMatchmakingServers() {
	return NULL;
}

S_API void *SteamNetworking() {
	return NULL;
}

S_API void *SteamRemoteStorage() {
	return NULL;
}

S_API void *SteamScreenshots() {
	return NULL;
}

S_API ISteamUser *SteamUser() {
	return NULL;
}

S_API void *SteamUserStats() {
	return NULL;
}

S_API ISteamUtils *SteamUtils() {
	return NULL;
}

S_API ISteamGameServer* SteamGameServer() {
	return NULL;
}

S_API ISteamUtils* SteamGameServerUtils() {
	return NULL;
}

S_API int SteamGameServer_GetHSteamPipe() {
	return 0;
}

S_API int SteamGameServer_GetHSteamUser() {
	return 0;
}

S_API int SteamGameServer_GetIPCCallCount() {
	return 0;
}

S_API bool SteamGameServer_Init(uint32 unIP, uint16 usPort, uint16 usGamePort, void* eServerMode, int nGameAppId, const char* pchGameDir, const char* pchVersionString) {
	NOTE_UNUSED(unIP);
	NOTE_UNUSED(usPort);
	NOTE_UNUSED(usGamePort);
	NOTE_UNUSED(eServerMode);
	NOTE_UNUSED(nGameAppId);
	NOTE_UNUSED(pchGameDir);
	NOTE_UNUSED(pchVersionString);

	return false;
}

S_API int SteamGameServer_InitSafe() {
	return 0;
}

S_API void SteamGameServer_RunCallbacks() {
}

S_API void SteamGameServer_Shutdown() {
}
