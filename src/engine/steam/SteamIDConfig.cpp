#include "tier0/dbg.h"
#include "color.h"
#include "steamCommon.h"
#include "steam/steamclientpublic.h"
#include "steam/steam_api.h"
#include "SteamIDConfig.h"
#include "RevEmu.h"
#include "ExternIP.h"

#include "sys_dll.h"
#include <ctime>

static bool g_bSidCfg_FirstStart = true;
extern int g_iSteamAppID;

/*
* Generate account id using an external ip
* 
* @output       Account ID as an integer.
*/
int get_accountid()
{
	int Ip = GetExternalIPLong();
	int First = (Ip >> 24) & 0xFF;
	int Second = (Ip >> 16) & 0xFF;
	int Third = 0;
	int Fourth = 0;

	int result = 0;

	for (int i = 0; i < 256; i++) {
		if (i == 255) {
			Third /= 256;
			Fourth /= 256;
		}

		Third += i;
		Fourth += i;
	}

	result = (int)(((unsigned int)First << 24) |
		((unsigned int)Second << 16) |
		((unsigned int)Third << 8) |
		(unsigned int)Fourth);

	if (result < 0) result *= -1;

	do {
		result -= 4;
	} while (result > INT_MAX);

	return result;
}

SteamIDConfig::SteamIDConfig() : steamID(0) {
	if (g_bSidCfg_FirstStart) {
		srand((unsigned)_time64(0));
		
		if (!SteamUser())
			steamID = get_accountid();
		else
			steamID = SteamUser()->GetSteamID().GetAccountID() / 2;

		g_bSidCfg_FirstStart = false;

		ConColorMsg(Color(100, 255, 100, 255), "[SteamIDConfig] ");
		Msg("Using SteamID: STEAM_0:0:%i\n", steamID); 
	}
	else {
		steamID = get_accountid();
	}
}

SteamIDConfig::~SteamIDConfig() {
	steamID = 0;
}

int SteamIDConfig::CreateTicket(void* pData, CSteamID sid, uint32 ip, uint16 port, bool secure) {
	char Gen[16];

	if (SteamUser())
		Ticket = SteamUser()->InitiateGameConnection(pData, 2048, sid, CGameID(g_iSteamAppID), ntohl(ip), ntohs(port), secure);
	else
		Ticket = GenerateRevEmu(pData, steamID, 4); // spoof the ticket if SteamUser doesn't exist

	ConColorMsg(Color(100, 255, 100, 255), "[SteamIDConfig] ");
	Msg("Created ticked for %s ", GetEmulatorName());

	auto pTicket = (int*)pData;
	auto pbTicket = (uint8*)pData;

	strcpy(Gen, "RevEmu 3 Gen"); // mostly popular on v34 so set this as default

	if (pTicket[0] == 'S')
		strcpy(Gen, "RevEmu 4 Gen");

	Msg("(%s)\n", Gen);
	
	ConColorMsg(Color(100, 255, 100, 255), "[SteamIDConfig] ");
	Msg("SteamID: %i\n", (pTicket[4] >> 1));

	return Ticket;
}

const char* SteamIDConfig::GetEmulatorName() {
	char result[MAX_PATH];
	result[0] = 0;
	strcpy(result, "None");

	if (Ticket == 152)
		strcpy(result, "RevEmu 9.83+");

	if (Ticket == 178)
		strcpy(result, "RevCrew SteamClient2009");

	if (Ticket == 322)
		strcpy(result, "RevEmu (CSSv34 ClientMod)");

	if (Ticket == 768)
		strcpy(result, "SteamEmu");


	return result;
}
