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

#if 0
class CLanServers : public ILanServers
{
public:
	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet);
	void RequestServerList(const char* gamedir, IServerListResponse* response);
};

static CLanServers s_lanservers;
#endif

ILanServers* lanservers = 0; //(ILanServers*)&s_lanservers;