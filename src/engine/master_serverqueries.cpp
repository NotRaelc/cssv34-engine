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

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
typedef struct a2s_player_s
{
	struct player {
		int		m_nId;
		char	m_szName[32];
		long	m_nScore;
		float	m_flTime;
	};

	// Number of players
	long	m_nCount;
	player* players;

} a2s_player_t;

struct query_t
{
	EServerQuery type;
	IServerPingResponse* ping_response;
	IServerPlayersResponse* players_response;
};

class CServerQueriesMaster : public IServerQueriesMaster
{
public:
	CServerQueriesMaster();
	~CServerQueriesMaster();

	void RunFrame(void);
	void ProcessConnectionlessPacket(netpacket_t* packet);

	newgameserver_t& ProcessInfo(bf_read& buf);
	void RequestServerInfo(const netadr_t& adr);

	a2s_player_t& ProcessPlayers(bf_read& buf);
	void RequestPlayers(const netadr_t& adr);

	query_t FindQuery(EServerQuery type, const netadr_t& adr);
	bool IsValidQuery(EServerQuery type, const netadr_t& adr);
	void CreateQuery(uint32 unIP, uint16 usPort, EServerQuery type, IServerPingResponse* pi_response, IServerPlayersResponse* pl_response);

	void PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* response);
	void PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* response);
	bool CancelServerQuery(EServerQuery type, uint32 unIP, uint16 usPort);

private:
	long m_nChallengeNumber;

	CUtlMap<netadr_t, query_t> m_serverQueries;
};

static CServerQueriesMaster s_serverqueries;
IServerQueriesMaster *serverqueries = (IServerQueriesMaster*)&s_serverqueries;

CServerQueriesMaster::CServerQueriesMaster()
{
	m_nChallengeNumber = -1;
	SetDefLessFunc(m_serverQueries);
}

CServerQueriesMaster::~CServerQueriesMaster()
{

}

void CServerQueriesMaster::RunFrame(void) {

}

void CServerQueriesMaster::ProcessConnectionlessPacket(netpacket_t* packet) {
	bf_read msg = packet->message;
	byte c = msg.ReadByte();

	if (c == 0)
		return;

	Msg("ServerQueriesMaster: Got connectionless packet (%c) from %s\n", c, packet->from.ToString());

	switch (c) {
	case S2A_INFOREPLY:
	{
		newgameserver_t& s = ProcessInfo(msg);

		query_t query = FindQuery(k_ePingServer, packet->from);

		if (!query.ping_response)
			return;

		//s.m_nPing = (Plat_FloatTime() - requestTime) * 1000.0; // calculate ping here

		s.m_NetAdr = packet->from;

		query.ping_response->ServerResponded(s);
		CancelServerQuery(query.type, packet->from.GetIPHostByteOrder(), packet->from.GetPort());

		break;
	}
	case S2C_CHALLENGE:
	{
		int challenge = msg.ReadLong();
		//if (challenge == -1 || challenge == 0)
		//	break;

		// Save our challenge
		m_nChallengeNumber = challenge;
		Msg("ServerQueriesMaster: Got challenge number (%i)\n", challenge);

		// check what this challenge actually used for
		query_t query = FindQuery(k_ePlayerDetails, packet->from);

		// retry request
		if (query.type != 0)
		{
			Msg("ServerQueriesMaster: Retrying players request with saved challenge number.\n");
			RequestPlayers(packet->from);
		}

		break;
	}

	case S2A_PLAYER_REPLY:
	{
		a2s_player_t result = ProcessPlayers(msg);
		query_t query = FindQuery(k_ePlayerDetails, packet->from);

		if (query.type == 0) {
			Warning("Wrong request type (expected %i, got %i)\n", k_ePlayerDetails, query.type);
			return;
		}

		if (!query.players_response) {
			Warning("NULL players response\n");
			return;
		}

		if (result.m_nCount == 0)
		{
			query.players_response->PlayersFailedToRespond();
			CancelServerQuery(query.type, packet->from.GetIPHostByteOrder(), packet->from.GetPort());
			break;
		}

		for (int i = 0; i < result.m_nCount; i++) {
			query.players_response->AddPlayerToList(result.players[i].m_szName, result.players[i].m_nScore, result.players[i].m_flTime);
		}

		query.players_response->PlayersRefreshComplete();
		Msg("ServerQueriesMaster: players refresh complete.\n");
		CancelServerQuery(query.type, packet->from.GetIPHostByteOrder(), packet->from.GetPort());

		break;
	}
	}
}

newgameserver_t& CServerQueriesMaster::ProcessInfo(bf_read& buf)
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

	return s;
}

void CServerQueriesMaster::RequestServerInfo(const netadr_t& adr)
{
	char string[256];
	bf_write msg(string, sizeof(string));

	msg.WriteLong(CONNECTIONLESS_HEADER); // 0xFFFFFFFF
	msg.WriteByte(A2S_INFOREQUEST); // 'T'
	msg.WriteString(A2S_KEY_STRING); // "Source Engine Query"
	msg.WriteByte(0);

	MasterNetHandler()->NET_SendPacket(NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten());
}


a2s_player_t& CServerQueriesMaster::ProcessPlayers(bf_read& buf) {
	static a2s_player_t playerQuery;
	memset(&playerQuery, 0, sizeof(playerQuery));

	int numClients = buf.ReadByte();
	if (numClients <= 0)
	{
		//m_serverPlayersResponse->PlayersFailedToRespond();
		return playerQuery;
	}

	playerQuery.m_nCount = numClients;
	playerQuery.players = new a2s_player_t::player[numClients];

	// chunks starting
	for (int i = 0; i < numClients; i++) {
		playerQuery.players[i].m_nId = buf.ReadByte();
		buf.ReadString(playerQuery.players[i].m_szName, sizeof(playerQuery.players[i].m_szName));
		playerQuery.players[i].m_nScore = buf.ReadLong();
		playerQuery.players[i].m_flTime = buf.ReadFloat();

		//ConColorMsg(Color(100, 255, 100, 255), "ServerQueriesMaster: processed player { id:%u, name:\"%s\", score:%i, time:%f }\n",
		//	playerQuery.players[i].m_nId,
		//	playerQuery.players[i].m_szName,
		//	playerQuery.players[i].m_nScore,
		//	playerQuery.players[i].m_flTime
		//);
	}

	//m_serverPlayersResponse->PlayersRefreshComplete();
	return playerQuery;
}

void CServerQueriesMaster::RequestPlayers(const netadr_t& adr)
{
	char string[256];
	bf_write msg(string, sizeof(string));

	msg.WriteLong(CONNECTIONLESS_HEADER); // 0xFFFFFFFF
	msg.WriteByte(A2S_PLAYER_REQUEST); // 'T'
	msg.WriteLong(m_nChallengeNumber);
	msg.WriteByte(0);

	MasterNetHandler()->NET_SendPacket(NS_CLIENT, adr, msg.GetData(), msg.GetNumBytesWritten());
}

query_t CServerQueriesMaster::FindQuery(EServerQuery type, const netadr_t& adr)
{
	query_t nullQuery = { (EServerQuery)0, 0, 0 };

	FOR_EACH_MAP(m_serverQueries, i)
	{
		if (i == m_serverQueries.InvalidIndex())
			continue;

		query_t& query = m_serverQueries[i];
		netadr_t& key = m_serverQueries.Key(i);
		if (key == adr)
		{
			if (type == k_eQuery_Any)
				return query;

			if (query.type == type)
				return query;
		}
	}
	return nullQuery;
}

bool CServerQueriesMaster::IsValidQuery(EServerQuery type, const netadr_t& adr) {
	FOR_EACH_MAP(m_serverQueries, i)
	{
		if (i == m_serverQueries.InvalidIndex())
			continue;

		query_t& query = m_serverQueries[i];
		netadr_t& key = m_serverQueries.Key(i);
		if (key == adr)
		{
			if (type == k_eQuery_Any)
				return true;

			if (query.type == type)
				return true;
		}
	}

	return false;
}

void CServerQueriesMaster::CreateQuery(uint32 unIP, uint16 usPort, EServerQuery type, IServerPingResponse* pi_response, IServerPlayersResponse* pl_response) {
	netadr_t addr(unIP, usPort);
	query_t query;
	memset(&query, 0, sizeof(query));

	query.type = type;
	query.ping_response = pi_response;
	query.players_response = pl_response;

	m_serverQueries.Insert(addr, query);

	if (type == k_ePingServer)
		RequestServerInfo(addr);
	else if (type == k_ePlayerDetails)
		RequestPlayers(addr);

	Msg("ServerQueriesMaster: Created server query: type=%i, address=%s\n", type, addr.ToString());
}

void CServerQueriesMaster::PingServer(uint32 unIP, uint16 usPort, IServerPingResponse* response) {
	CreateQuery(unIP, usPort, k_ePingServer, response, 0);
}

void CServerQueriesMaster::PlayerDetails(uint32 unIP, uint16 usPort, IServerPlayersResponse* response) {
	CreateQuery(unIP, usPort, k_ePlayerDetails, 0, response);
}

bool CServerQueriesMaster::CancelServerQuery(EServerQuery type, uint32 unIP, uint16 usPort) {
	netadr_t addr(unIP, usPort);

	FOR_EACH_MAP(m_serverQueries, i)
	{
		if (m_serverQueries[i].type != type)
			continue;

		unsigned short index = m_serverQueries.Find(addr);

		if (index == m_serverQueries.InvalidIndex())
			break;

		if (index == i) {
			m_serverQueries.RemoveAt(index);
			return true;
		}
	}

	return false;
}