#include <WinSock2.h>
#include "quakedef.h"
#include "server.h"
#include "master.h"
#include "proto_oob.h"
#include "host.h"
#include "eiface.h"
#include "server.h"
#include "cdll_int.h"
#include "utlmap.h"
#include "utlbuffer.h"
#include "sys_dll.h"

#define FAVORITES_REFRESH_TIMEOUT 10.0

class СHistoryServers : public IServerList, public IServerPingResponse
{
public:
	СHistoryServers();
	virtual ~СHistoryServers();

	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet) {}
	void RequestServerList(const char* gamedir, IServerListResponse* response);

	void AddServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed = 0ull);
	void RemoveServer(uint32 unIP, uint16 usPort);

	void List_AddServer(KeyValues* kv);
	void List_RemoveServer(netadr_t* addr);
	void List_UpdateServer(KeyValues* updatedData);
	void List_UpdateServers();

	void ServerResponded(newgameserver_t& server);

	void StopRefresh();

private:
	char m_szGameDir[64];
	bool m_bRefreshing;
	int m_iServersCount;
	float m_flStartRequestTime;
	IServerListResponse* m_serverListResponse;
};

static СHistoryServers s_history_servers;

IServerList* historyservers = (IServerList*)&s_history_servers;

СHistoryServers::СHistoryServers()
{
	m_bRefreshing = false;
	m_iServersCount = 0;
	m_serverListResponse = 0;
	m_flStartRequestTime = 0;
}

СHistoryServers::~СHistoryServers()
{

}

void СHistoryServers::RunFrame() {
	if (!m_bRefreshing)
		return;

	if (m_iServersCount <= 0 &&
		m_flStartRequestTime < Plat_FloatTime() - FAVORITES_REFRESH_TIMEOUT) {
		StopRefresh();
		m_serverListResponse->RefreshComplete(nNoServersListedOnMasterServer);
		return;
	}

	if (m_iServersCount > 0 &&
		m_flStartRequestTime < Plat_FloatTime() - FAVORITES_REFRESH_TIMEOUT) {
		StopRefresh();
		m_serverListResponse->RefreshComplete(nServerResponded);
		return;
	}
}


void СHistoryServers::StopRefresh()
{
	if (!m_bRefreshing)
		return;

	m_iServersCount = 0;
	m_bRefreshing = false;
}

void СHistoryServers::AddServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed)
{
	m_iServersCount++;
	char serverID[16];
	Q_snprintf(serverID, sizeof(serverID), "%d", m_iServersCount);

	KeyValues* server = new KeyValues(serverID);
	netadr_t addr(unIP, usPort);

	server->SetString("name", "unknown");
	server->SetString("gamedir", "unknown");
	server->SetInt("players", 0);
	server->SetInt("maxplayers", 0);
	server->SetString("map", "unknown");
	server->SetString("address", addr.ToString());
	server->SetUint64("lastplayed", timeLastPlayed);

	Msg("Added server %s to history\n", addr.ToString());

	List_AddServer(server);
}

void СHistoryServers::RemoveServer(uint32 unIP, uint16 usPort)
{
	netadr_t addr(unIP, usPort);
	List_RemoveServer(&addr);
}

void СHistoryServers::List_AddServer(KeyValues* kv)
{
	KeyValues* savedData = new KeyValues("Filters");

	savedData->LoadFromFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG");

	KeyValues* favorites = savedData->FindKey("History", true);

	favorites->AddSubKey(kv);

	savedData->SaveToFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG");
	savedData->deleteThis();

	List_UpdateServers();
}

void СHistoryServers::List_RemoveServer(netadr_t* addr) {
	KeyValues* savedData = new KeyValues("Filters");

	savedData->LoadFromFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG");

	KeyValues* favorites = savedData->FindKey("History", true);

	for (KeyValues* dat = favorites->GetFirstSubKey(); dat; dat = dat->GetNextKey())
	{
		if (!Q_stricmp(dat->GetString("address"), addr->ToString()))
		{
			favorites->RemoveSubKey(dat);
		}
	}

	savedData->SaveToFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG");
	savedData->deleteThis();

	List_UpdateServers();
}

void СHistoryServers::List_UpdateServer(KeyValues* updatedData)
{
	KeyValues* savedData = new KeyValues("");

	if (!savedData->LoadFromFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG"))
		return;

	KeyValues* favorites = savedData->FindKey("History", true);

	for (KeyValues* dat = favorites->GetFirstSubKey(); dat; dat = dat->GetNextKey())
	{
		if (!Q_stricmp(dat->GetString("address"), updatedData->GetString("address")))
		{
			Msg("Updating server %s in history\n", updatedData->GetString("address"));
			dat->SetString("name", updatedData->GetString("name"));
			dat->SetString("gamedir", updatedData->GetString("gamedir"));
			dat->SetInt("players", updatedData->GetInt("players"));
			dat->SetInt("maxplayers", updatedData->GetInt("maxplayers"));
			dat->SetString("map", updatedData->GetString("map"));
			break;
		}
	}

	savedData->SaveToFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG");
	savedData->deleteThis();
}


void СHistoryServers::List_UpdateServers()
{
	KeyValues* savedData = new KeyValues("");

	if (!savedData->LoadFromFile(g_pFullFileSystem, "serverbrowser.vdf", "CONFIG"))
		return;

	KeyValues* favorites = savedData->FindKey("History", true);

	m_iServersCount = 0;

	for (KeyValues* dat = favorites->GetFirstSubKey(); dat; dat = dat->GetNextKey())
	{
		const char* addrStr = dat->GetString("address");
		if (!addrStr || !addrStr[0])
			continue;

		netadr_t addr(addrStr);

		m_iServersCount++;

		g_pServersInfo->PingServer(addr.GetIPHostByteOrder(), addr.GetPort(), this);
	}

	savedData->deleteThis();
}

void СHistoryServers::ServerResponded(newgameserver_t& server) {
	if (m_serverListResponse && m_bRefreshing) {
		Msg("History server %s responded.\n", server.m_NetAdr.ToString());
		m_serverListResponse->ServerResponded(server);
	}

	Msg("Got response from History server.\n%s\n", server.toString());

	KeyValues* dat = new KeyValues("server");

	dat->SetString("name", server.m_szServerName);
	dat->SetString("gamedir", server.m_szGameDir);
	dat->SetInt("players", server.m_nPlayers);
	dat->SetInt("maxplayers", server.m_nMaxPlayers);
	dat->SetString("map", server.m_szMap);

	dat->SetString("address", server.m_NetAdr.ToString());

	List_UpdateServer(dat);

	dat->deleteThis();
}

void СHistoryServers::RequestServerList(const char* gamedir, IServerListResponse* response)
{
	m_serverListResponse = response;
	m_bRefreshing = true;
	m_flStartRequestTime = Plat_FloatTime();

	Q_strncpy(m_szGameDir, gamedir, sizeof(m_szGameDir));

	List_UpdateServers();
}