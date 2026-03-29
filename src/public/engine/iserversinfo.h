//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to steam managing game server/client match making
//
//=============================================================================

#ifndef ISERVERSINFO_H
#define ISERVERSINFO_H
#ifdef _WIN32
#pragma once
#endif

enum NServerResponse
{
	nServerResponded = 0,
	nServerFailedToRespond,
	nNoServersListedOnMasterServer,
};

class newgameserver_t
{
public:
	newgameserver_t() = default;

	netadr_t m_NetAdr;						///< IP/Query Port/Connection Port for this server
	int m_nPing;							///< current ping time in milliseconds
	int m_nProtocolVersion;
	bool m_bHadSuccessfulResponse;			///< server has responded successfully in the past
	bool m_bDoNotRefresh;					///< server is marked as not responding and should no longer be refreshed
	char m_szGameDir[64];					///< current game directory
	char m_szMap[64];						///< current map
	char m_szGameTags[128];
	char m_szGameDescription[128];			///< game description
	int m_nAppID;
	int m_nPlayers;
	int m_nMaxPlayers;						///< Maximum players that can join this server
	int m_nBotPlayers;						///< Number of bots (i.e simulated players) on this server
	bool m_bPassword;						///< true if this server needs a password to join
	bool m_bSecure;							///< server uses VAC (Valve Anti-Cheat)
	char m_szGameVersion[64];

	int m_iFlags;

	/// Game server name
	char m_szServerName[256];
};

struct FilterPair_t
{
	FilterPair_t() { m_szKey[0] = m_szValue[0] = 0; }
	FilterPair_t(const char* pchKey, const char* pchValue)
	{
		strncpy(m_szKey, pchKey, sizeof(m_szKey)); // this is a public header, use basic c library string funcs only!
		strncpy(m_szValue, pchValue, sizeof(m_szValue));
	}
	char m_szKey[256];
	char m_szValue[256];
};

class IServerListResponse
{
public:
	// Server has responded ok with updated data
	virtual void ServerResponded(newgameserver_t& server) = 0;
	virtual void RefreshComplete(NServerResponse response) = 0;
};

//-----------------------------------------------------------------------------
// Purpose: Callback interface for receiving responses after pinging an individual server 
//
class IServerPingResponse
{
public:
	// Server has responded successfully and has updated data
	virtual void ServerResponded(newgameserver_t& server) = 0;
};
//-----------------------------------------------------------------------------
// Purpose: Callback interface for receiving responses after requesting details on
// who is playing on a particular server.
//
class IServerPlayersResponse
{
public:
	// Got data on a new player on the server -- you'll get this callback once per player
	// on the server which you have requested player data on.
	virtual void AddPlayerToList(const char* pchName, int nScore, float flTimePlayed) = 0;

	// The server failed to respond to the request for player details
	virtual void PlayersFailedToRespond() = 0;

	// The server has finished responding to the player details request
	virtual void PlayersRefreshComplete() = 0;
};

// Unique identificator for all server queries
typedef uint64 ServerQuery;

//-----------------------------------------------------------------------------
// Purpose: Functions for match making services for clients to get to game lists and details
//-----------------------------------------------------------------------------
class IServersInfo
{
public:
	virtual void RequestInternetServerList(const char* gamedir, IServerListResponse* response) = 0;
	virtual void RequestLANServerList(const char* gamedir, IServerListResponse* response) = 0;
	virtual void StopRefresh() = 0;

	virtual ServerQuery PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* response) = 0;
	virtual ServerQuery PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* response) = 0;
	virtual void		CancelServerQuery(ServerQuery ServerQuery) = 0;
};

// First interface version
#define SERVERLIST_INTERFACE_VERSION_PREVIOUS "ServerList001"

// Current interface version
#define SERVERLIST_INTERFACE_VERSION "ServerList002"

extern IServersInfo* g_pServersInfo;

#endif // ISERVERSINFO_H
