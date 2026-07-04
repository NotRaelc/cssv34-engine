#include <WinLite.h>
#include "quakedef.h"
#include "sys_dll.h"
#include "dbg.h"
#include "color.h"
#include "steam/http.h"
#include "master.h"
#include "../thirdparty/nlohmann/json.hpp"

#define HTTP_REQUEST_TIMEOUT 5.0 // seconds

using json = nlohmann::json;

const char* g_pszApiURLs[] = {
	"https://api.gamemonitoring.net/servers",
	"https://gamemonitoring.net/api/servers"
};

struct serverFilter
{
	serverFilter() { pchName[0] = pchValue[0] = 0; }
	serverFilter(const char* name, const char* value) {
		strcpy(pchName, name);
		strcpy(pchValue, value);
	}
	char pchName[64];
	char pchValue[32];
};

/*
* get server list from gamemonitoring api
* -----------
* @input filters, filtercount
* @output JSON
*/
char* get_servers(serverFilter* filters, int filtercount)
{
	Warning("get_servers\n");
	int iResSize = 0;
	int iRetCode = 0;
	void* pData = nullptr;

	char url[2048] = {};
	for (int i = 0; i < sizeof(g_pszApiURLs) / sizeof(*g_pszApiURLs); i++)
	{
		sprintf(url, "%s?", g_pszApiURLs[i]);
		for (int a = 0; a < filtercount; a++)
		{
			strcat(url, filters[a].pchName);
			strcat(url, "=");
			strcat(url, filters[a].pchValue);
			if (filtercount == 1 && a == 0)
				break;

			strcat(url, "&");
		}

		Msg("Request URL: %s\n", url);

		pData = HTTPGetRequest(url, &iResSize, &iRetCode);

		Msg(
			"HTTP result: code=%d size=%d ptr=%p\n",
			iRetCode,
			iResSize,
			pData);

		if (pData && iResSize && iRetCode == 200)
			break;
	}

	if (!pData || !iResSize || iRetCode != 200)
	{
		Warning("Request Error at get_servers.\nURL: % s\nReturn code : % i\n", url, iRetCode);
		return nullptr;
	}

	static std::string jsonBuffer;

	jsonBuffer.assign(
		(const char*)pData,
		iResSize
	);

	free(pData);

	Plat_DebugString(jsonBuffer.c_str());
	Plat_DebugString("\n");

	return (char*)jsonBuffer.c_str();
}

/*
* get information about server
* @input serverID
* @output JSON
*/
char* get_server(uint32 id)
{
	//Warning("get_server\n");
	int iResSize = 0;
	int iRetCode = 0;
	void* pData = nullptr;

	char url[2048] = {};
	for (int i = 0; i < sizeof(g_pszApiURLs) / sizeof(*g_pszApiURLs); i++)
	{
		sprintf(url, "%s/%u", g_pszApiURLs[i], id);

		//Msg("Request URL: %s\n", url);

		pData = HTTPGetRequest(url, &iResSize, &iRetCode);

		//Msg(
		//	"HTTP result: code=%d size=%d ptr=%p\n",
		//	iRetCode,
		//	iResSize,
		//	pData);

		if (pData && iResSize && iRetCode == 200)
			break;
	}

	if (!pData || !iResSize || iRetCode != 200)
	{
		Warning("Request Error at get_server.\nURL: % s\nReturn code : % i\n", url, iRetCode);
		return nullptr;
	}

	static std::string jsonBuffer;

	jsonBuffer.assign(
		(const char*)pData,
		iResSize
	);

	free(pData);

	Plat_DebugString(jsonBuffer.c_str());
	Plat_DebugString("\n");

	return (char*)jsonBuffer.c_str();
}

//
// gameMonitoring.net server list
//
class CGameMonitoringServerList : public IServerList
{
public:
	CGameMonitoringServerList();
	~CGameMonitoringServerList();

	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet) {}
	void RequestServerList(const char* gamedir, IServerListResponse* response);

	void AddServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed = 0ull) {}
	void RemoveServer(uint32 unIP, uint16 usPort) {}

	void StopRefresh();

protected:
	static void RequestServers(CGameMonitoringServerList* This);

private:
	float m_flStartRequestTime;
	bool m_bRefreshing;

	int m_iServersResponded;

	HANDLE m_hThread;

	IServerListResponse* m_serverListResponse;
};

static CGameMonitoringServerList s_monitoringservers;
IServerList* monitoringservers = (IServerList*)&s_monitoringservers;

CGameMonitoringServerList::CGameMonitoringServerList() {
	m_bRefreshing = 0;
	m_serverListResponse = 0;
	m_iServersResponded = 0;
	m_hThread = 0;
}

CGameMonitoringServerList::~CGameMonitoringServerList() {

}

void CGameMonitoringServerList::RunFrame() {
	if (!m_bRefreshing)
		return;

	if (m_serverListResponse &&
		m_flStartRequestTime < Plat_FloatTime() - HTTP_REQUEST_TIMEOUT)
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete(NServerResponse::nServerFailedToRespond);
		return;
	}

	if (m_iServersResponded > 0 &&
		m_flStartRequestTime < Plat_FloatTime() - HTTP_REQUEST_TIMEOUT)
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete(NServerResponse::nServerResponded);
		return;
	}
}

void CGameMonitoringServerList::StopRefresh()
{
	if (!m_bRefreshing)
		return;

	m_iServersResponded = 0;
	m_bRefreshing = false;
}


void CGameMonitoringServerList::RequestServerList(const char* gamedir, IServerListResponse* response) {
	Warning("CGameMonitoringServerList::RequestServerList\n");
	if (response) {
		m_bRefreshing = true;
		m_flStartRequestTime = Plat_FloatTime();
		m_serverListResponse = response;
	}

	m_hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RequestServers, this, 0, 0);
}

void CGameMonitoringServerList::RequestServers(CGameMonitoringServerList* This)
{
	if (!This->m_bRefreshing)
		return;

	char game[32];
	itoa(GetSteamInfIDVersionInfo().AppID, game, 10);

	serverFilter filters[] = {
		{"game", game},
		{"version", GetSteamInfIDVersionInfo().szVersionString},
		{"limit", "384"}
	};

	char* raw_response = get_servers(filters, sizeof(filters) / sizeof(*filters));
	if (!raw_response) {
		This->m_serverListResponse->RefreshComplete(nServerFailedToRespond);
		return;
	}

	json root = json::parse(raw_response);

	if (!root.contains("response"))
	{
		This->m_serverListResponse->RefreshComplete(nServerFailedToRespond);
		return;
	}

	json& response = root["response"];

	//Msg("Has response: %d\n", root.contains("response"));
	//Msg("Has items: %d\n", response.contains("items"));
	//Msg("Items count: %u\n", response["items"].size());

	for (const auto& item : response["items"]) {

		if (!This->m_bRefreshing)
			break;

		newgameserver_t server{};

		// get detailed information of server
		char* raw_sv = get_server(item["id"]);
		json sv_response = json::parse(raw_sv);
		json& sv = sv_response["response"];
		
		// Sometimes address is invalid
		try
		{
			server.m_NetAdr.SetFromString(sv["connect"].get<std::string>().c_str());
		}
		catch (std::exception e)
		{
			Msg("Exception in " __FUNCTION__ ", line " V_STRINGIFY(__LINE__) ": %s\n", e.what());
			continue;
		}

		strcpy(server.m_szGameDir, GetSteamInfIDVersionInfo().szProductString);
		strcpy(server.m_szMap, sv["map"].get<std::string>().c_str());
		strcpy(server.m_szGameDescription, sv["gamemode"].get<std::string>().c_str());
		strcpy(server.m_szServerName, sv["name"].get<std::string>().c_str());
		strcpy(server.m_szGameVersion, sv["version"].get<std::string>().c_str());

		server.m_nProtocolVersion = 7;
		server.m_nAppID = sv["game"];
		server.m_bSecure = sv["secured"];
		server.m_bPassword = sv["private"];
		server.m_nPlayers = sv["numplayers"];
		server.m_nBotPlayers = sv["bots"];
		server.m_nMaxPlayers = sv["maxplayers"];

		This->m_serverListResponse->ServerResponded(server);
		//Msg("MonitoringServerList: Added %s\n", server.toString());
	}

	Warning("MonitoringServerList: Refresh complete\n");
	This->m_serverListResponse->RefreshComplete(nServerResponded);

	return;
}