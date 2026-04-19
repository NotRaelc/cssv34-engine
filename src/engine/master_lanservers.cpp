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

#define INFO_REQUEST_TIMEOUT 5.0 // seconds

class CLanServers : public IServerList
{
public:
	CLanServers();
	virtual ~CLanServers();

	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet);
	void RequestServerList(const char* gamedir, IServerListResponse* response);

	void AddServer(uint32 unIP, uint16 usPort, time_t timeLastPlayed = 0ull) {}
	void RemoveServer(uint32 unIP, uint16 usPort) {}

	void StopRefresh();

	void				RequestInfo();
	newgameserver_t&	ProcessInfo(bf_read& buf);

private:
	char m_szGameDir[64];

	float m_flStartRequestTime;
	bool m_bRefreshing;

	int m_iServersResponded;

	IServerListResponse* m_serverListResponse;
};

static CLanServers s_lanservers;

IServerList* lanservers = (IServerList*)&s_lanservers; //(ILanServers*)&s_lanservers;

CLanServers::CLanServers()
{
	m_bRefreshing = 0;
	m_serverListResponse = 0;
	m_iServersResponded = 0;
}

CLanServers::~CLanServers()
{

}

void CLanServers::RequestInfo() {
	char buf[256];
	bf_write msg(buf, sizeof(buf));

	msg.WriteLong(CONNECTIONLESS_HEADER); // 0xFFFFFFFF
	msg.WriteByte(A2S_INFOREQUEST); // 'T'
	msg.WriteString(A2S_KEY_STRING); // "Source Engine Query"

	netadr_t to;
	to.type = NA_BROADCAST;

	Msg("LanServers: requesting servers info.\n");
	for (uint16 port = 27000; port < 27050; port++)
	{
		to.port = ntohs(port);
		MasterNetHandler()->NET_SendPacket(NS_CLIENT, to, msg.GetData(), msg.GetNumBytesWritten());
	}

}

newgameserver_t& CLanServers::ProcessInfo(bf_read& buf)
{
	static newgameserver_t s;
	memset(&s, 0, sizeof(s));

	s.m_nProtocolVersion = buf.ReadByte();

	buf.ReadString(s.m_szServerName, sizeof(s.m_szServerName));
	buf.ReadString(s.m_szMap, sizeof(s.m_szMap));
	buf.ReadString(s.m_szGameDir, sizeof(s.m_szGameDir));

	buf.ReadString(s.m_szGameDescription, sizeof(s.m_szGameDescription));
	s.m_nAppID = buf.ReadShort();

	// player info
	s.m_nPlayers = buf.ReadByte();
	s.m_nMaxPlayers = buf.ReadByte();
	s.m_nBotPlayers = buf.ReadByte();

	// Password?
	buf.ReadByte(); // server type
	buf.ReadByte(); // env

	s.m_bPassword = buf.ReadByte();
	s.m_bSecure = buf.ReadByte();
	buf.ReadString(s.m_szGameVersion, sizeof(s.m_szGameVersion));

	s.m_iFlags = buf.ReadLong();

	return s;
}

void CLanServers::RunFrame()
{
	if (!m_bRefreshing)
		return;

	if (m_serverListResponse &&
		m_flStartRequestTime < Plat_FloatTime() - INFO_REQUEST_TIMEOUT)
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete(NServerResponse::nServerFailedToRespond);
		return;
	}

	if (m_iServersResponded > 0 &&
		m_flStartRequestTime < Plat_FloatTime() - INFO_REQUEST_TIMEOUT)
	{
		StopRefresh();
		m_serverListResponse->RefreshComplete(NServerResponse::nServerResponded);
		return;
	}
}

void CLanServers::StopRefresh()
{
	if (!m_bRefreshing)
		return;

	m_iServersResponded = 0;
	m_bRefreshing = false;
}

void CLanServers::ProcessConnectionlessPacket(netpacket_t* packet) {
	bf_read msg = packet->message;
	byte c = msg.ReadByte();

	switch (c)
	{

	case S2A_INFOREPLY:
	{
		if (!m_bRefreshing)
			break;

		Msg("LanServers: Got S2A_INFOREPLY from %s\n", packet->from.ToString());

		newgameserver_t& s = ProcessInfo(msg);
		s.m_nPing = 0;
		s.m_NetAdr = packet->from;

		if (strcmp(m_szGameDir, s.m_szGameDir)) {
			Msg("LanServers: wrong GameDir, our %s, server's %s\n", m_szGameDir, s.m_szGameDir);
			break;
		}

		Msg("LanServers: GameDir confirmed, our %s, server's %s\n", m_szGameDir, s.m_szGameDir);

		m_serverListResponse->ServerResponded(s);
		m_iServersResponded++;

		break;
	}

	}
}

void CLanServers::RequestServerList(const char* gamedir, IServerListResponse* response) {
	if (response) {
		m_bRefreshing = true;
		m_flStartRequestTime = Plat_FloatTime();
		m_serverListResponse = response;
	}

	strcpy(m_szGameDir, gamedir);

	RequestInfo();
}