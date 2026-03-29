#pragma once
#include "checksum_crc.h"
#include "strtools.h"
#include "steam/steamclientpublic.h"

// Special class that generates SteamID from External IP
class SteamIDConfig {
public:
	SteamIDConfig();
	~SteamIDConfig();

	int			CreateTicket(void* pData, CSteamID sid = 0ull, uint32 ip = 0u, uint16 port = 0u, bool secure = false);
	const char* GetEmulatorName();

private:
	int steamID;
	int Ticket;
};