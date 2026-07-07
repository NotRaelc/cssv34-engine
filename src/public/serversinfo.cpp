#include <winsock2.h>
#include "serversinfo.h"
#include "../tier1/interface.h"

#if defined(NOMASTER) && defined(ENGINE_DLL)
class CServersInfoStub : public IServersInfo
{
public:
	CServersInfoStub() {}
	virtual ~CServersInfoStub() {}

	virtual void RequestInternetServerList(const char* gamedir, IServerListResponse* response) { response->RefreshComplete(nNoServersListedOnMasterServer); }
	virtual void RequestLANServerList(const char* gamedir, IServerListResponse* response) { response->RefreshComplete(nNoServersListedOnMasterServer); }
	virtual void RequestFavoritesServerList(const char* gamedir, IServerListResponse* response) { response->RefreshComplete(nNoServersListedOnMasterServer); }
	virtual void RequestHistoryServerList(const char* gamedir, IServerListResponse* response) { response->RefreshComplete(nNoServersListedOnMasterServer); }
	virtual void StopRefresh() {}

	virtual void AddFavoriteServer(uint32 unIP, uint16 usPort) {}
	virtual void AddHistoryServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed) {}

	virtual void RemoveFavoriteServer(uint32 unIP, uint16 usPort) {}
	virtual void RemoveHistoryServer(uint32 unIP, uint16 usPort) {}

	virtual void PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* response) {}
	virtual void PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* response) {}
	virtual bool CancelServerQuery(EServerQuery type, uint32 unIP, uint16 usPort) { return true; }
};

static CServersInfoStub s_ServersInfoStub;

IServersInfo* g_pServersInfo = (IServersInfo*)&s_ServersInfoStub;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServersInfoStub, IServersInfo, SERVERLIST_INTERFACE_VERSION, s_ServersInfoStub);
#endif