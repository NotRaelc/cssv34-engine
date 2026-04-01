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

class CLanServers : public ILanServers
{
public:
	CLanServers();
	virtual ~CLanServers();

	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet);
	void RequestServerList(const char* gamedir, IServerListResponse* response);
private:
	char m_szGameDir[256];

	CUtlMap<netadr_t, bool> m_serverAddresses;
	CUtlMap<netadr_t, double> m_serversRequestTime;

	SOCKET m_nSocket;
};

static CLanServers s_lanservers;

ILanServers* lanservers = (ILanServers*)&s_lanservers; //(ILanServers*)&s_lanservers;

CLanServers::CLanServers()
{
	SetDefLessFunc(m_serverAddresses);
	SetDefLessFunc(m_serversRequestTime);
}

CLanServers::~CLanServers()
{

}

void CLanServers::RunFrame()
{
	char buf[256];
	bf_write msg(buf, sizeof(buf));

	msg.WriteLong(CONNECTIONLESS_HEADER); // 0xFFFFFFFF
	msg.WriteByte(A2S_INFOREQUEST); // 'T'
	msg.WriteString(A2S_KEY_STRING); // "Source Engine Query"

	netadr_t to;
	to.type = NA_BROADCAST;

	// Thank god this is running in masterNetHandler thread ^_^
	for (uint16 port = 27000; port < 27050; port++)
	{ 
		to.port = htons(port);
		MasterNetHandler()->NET_SendPacket(NS_CLIENT, to, msg.GetData(), msg.GetNumBytesWritten());
	}
}

void CLanServers::ProcessConnectionlessPacket(netpacket_t* packet) {

}

void CLanServers::RequestServerList(const char* gamedir, IServerListResponse* response) {

}