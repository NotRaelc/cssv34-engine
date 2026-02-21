#include "tier0/dbg.h"
#include "color.h"
#include "steamCommon.h"
#include "steam/steamclientpublic.h"
#include "steam/steam_api.h"
#include "SteamIDConfig.h"
#include "SC2009.h"
#include "ExternIP.h"
#include <ctime>

// To prevent another steamID generation by SteamIDConfig on runtime
static bool g_bSid_isAlreadyGenerated = false;
static unsigned int g_bSid_longIP = 0;

/*
 * Generates a random unsigned integer across its full range (2048 to UINT_MAX)
 * by combining results from multiple rand() calls.
 */
uint32 rand_low(void) {
	uint32 r = 0;

    r = (uint32)rand();
    if (sizeof(uint32) > sizeof(int)) {
        r = (r << (sizeof(int) * CHAR_BIT)) | (uint32)rand();
    }

	if (r < 2048)
		r += 2048;

	// r [2048 ; UINT32_MAX]

    return (uint32)r;
}

SteamIDConfig::SteamIDConfig() : steamID(), rawID(0) {
	if (!g_bSid_isAlreadyGenerated) {
		srand(_time64(0));

		g_bSid_longIP = (uint32)GetExternalIPLong();

		if (g_bSid_longIP > UINT_MAX)
			rawID = rand_low();
		else {
			rawID = (uint32)(g_bSid_longIP << 2); // if value exceeds uint32 limit, truncate to be uint32
		}

		steamID.Set(rawID, k_EUniversePublic, k_EAccountTypeIndividual);
		g_bSid_isAlreadyGenerated = true;

		// Notify us
		ConColorMsg(Color(100, 255, 100, 255), "[SteamIDConfig] ");
		CreateTicket(malloc(2048)); // create empty ticket to display what emulator are we using
		Msg("Ticket: %s\n", GetEmulatorName());

		ConColorMsg(Color(100, 255, 100, 255), "[SteamIDConfig] ");
		Msg("Using SteamID: STEAM_0:0:%u\n", rawID); // ok so X is always 0 (set in steamclientpublic.h), Y is also always 0 (because its bir3yk)
	}
	else {
		rawID = (uint32)(g_bSid_longIP << 2);
		steamID.Set(rawID, k_EUniversePublic, k_EAccountTypeIndividual);
		g_bSid_isAlreadyGenerated = true; // just in case, i know this is diabolic
	}
}

SteamIDConfig::~SteamIDConfig() {
	steamID = 0ull;
	rawID = 0u;
}

int SteamIDConfig::CreateTicket(void* pData) {
	Ticket = GenerateSC2009(pData, rawID);
	return Ticket;
}

const char* SteamIDConfig::GetEmulatorName() {
	char result[MAX_PATH];
	result[0] = 0;

	if (Ticket == 152)
		strcpy(result, "RevEmu 3rd Gen");

	if (Ticket == 178)
		strcpy(result, "RevEmu 4th Gen");

	if (Ticket == 194)
		strcpy(result, "RevEmu 2013");

	if (Ticket == 768)
		strcpy(result, "SteamEmu");


	return result;
}

CSteamID SteamIDConfig::GetStoredSteamID() {
	return steamID;
}