#pragma once
#include "checksum_crc.h"
#include "strtools.h"
#include "steam/steamclientpublic.h"

// Special class that generates SteamID from External IP
class SteamIDConfig {
public:
	SteamIDConfig();
	~SteamIDConfig();

	int			CreateTicket(void* pData);
	const char* GetEmulatorName();

	CSteamID	GetStoredSteamID();

private:
	CSteamID steamID;
	uint32 rawID;
	int Ticket;
};