//======== (C) Copyright 1999, 2000 Valve, L.L.C. All rights reserved. ========
//
// The copyright to the contents herein is the property of Valve, L.L.C.
// The contents may be used and/or copied only with the written permission of
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in
// the agreement/contract under which the contents have been supplied.
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================
#ifndef MASTER_H
#define MASTER_H
#ifdef _WIN32
#pragma once
#endif

#include "net.h"
#include "convar.h"
#include "engine/iserversinfo.h"

#undef CreateThread
#undef recvfrom
#undef sendto


//-----------------------------------------------------------------------------
// Purpose: Implements a master server interface.
//-----------------------------------------------------------------------------
class IMaster
{
public:
	// Allow master server to register cvars/commands
	virtual void Init( void ) = 0;
	// System is shutting down
	virtual void Shutdown( void ) = 0;
	// Server is shutting down
	virtual void ShutdownConnection( void ) = 0;
	// Sends the actual heartbeat to the master ( after challenge value is parsed )
	virtual void SendHeartbeat( struct adrlist_s *p ) = 0;
	// Add server to global master list
	virtual void AddServer( struct netadr_s *adr ) = 0;
	// If parsing for server, etc. fails, always have at least one server around to use.
	virtual void UseDefault ( void ) = 0;
	// See if it's time to send the next heartbeat
	virtual void CheckHeartbeat( void ) = 0;
	// Master sent back a challenge value, read it and send the actual heartbeat
	virtual void RespondToHeartbeatChallenge( netadr_t &from, bf_read &msg ) = 0;
	// Console command to set/remove master server
	virtual void AddMaster_f( const CCommand &args ) = 0;
	// Force a heartbeat to be issued right away
	virtual void Heartbeat_f( void ) = 0;
	// Processes connectionless packet
	virtual void ProcessConnectionlessPacket(netpacket_t* packet) = 0;
	// Runs every frame
	virtual void RunFrame( void ) = 0;
};

// Helper class for server queries
class IServerQueriesMaster
{
public:
	virtual void RunFrame(void) = 0;
	virtual void ProcessConnectionlessPacket(netpacket_t* packet) = 0;

	virtual void PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* response) = 0;
	virtual void PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* response) = 0;
	virtual bool CancelServerQuery(EServerQuery type, uint32 unIP, uint16 usPort) = 0;
};

/*
class IMasterNetwork
{
	// runs every frame, for some basic operations such as finding servers
	virtual void RunFrame(void) = 0;

	// returns array of the servers
	virtual void GetAllServers(newgameserver_t** servers) = 0;
};
*/

//-----------------------------------------------------------------------------
// Purpose: Implements server list.
//-----------------------------------------------------------------------------
class IServerList
{
public:
	virtual void RunFrame(void) = 0;
	virtual void ProcessConnectionlessPacket(netpacket_t* packet) = 0;
	virtual void RequestServerList(const char* gamedir, IServerListResponse* response) = 0;

	virtual void AddServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed = 0ull) = 0;
	virtual void RemoveServer(uint32 unIP, uint16 usPort) = 0;
};


//-----------------------------------------------------------------------------
// Purpose: Implements network functions for master server interfacs.
//-----------------------------------------------------------------------------
class IMasterNETHandler {
public:
	virtual void NET_SendPacket(int ns, const netadr_t& to, const byte* data, int length) = 0;
};

extern IMaster* master;
extern IServerQueriesMaster* serverqueries;
extern IServerList* lanservers;
extern IServerList* favoriteservers;
extern IServerList* historyservers;
extern IMasterNETHandler* MasterNetHandler();
extern IServersInfo* g_pServersInfo;

#endif // MASTER_H
