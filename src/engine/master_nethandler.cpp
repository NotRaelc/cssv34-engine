#include <WinLite.h>
#include <WinSock2.h>
#include "master.h"
#include "quakedef.h"

bool IsLANIP(uint32 ip)
{
	ip = ntohl(ip);

	// 10.0.0.0/8
	if ((ip & 0xFF000000) == 0x0A000000)
		return true;

	// 172.16.0.0 - 172.31.255.255
	if ((ip & 0xFFF00000) == 0xAC100000)
		return true;

	// 192.168.0.0/16
	if ((ip & 0xFFFF0000) == 0xC0A80000)
		return true;

	return false;
}

class CMasterNETHandler : public IMasterNETHandler {
public:
	CMasterNETHandler(void);
	~CMasterNETHandler(void);

	virtual void NET_SendPacket(int ns, const netadr_t& to, const byte* data, int length);
protected:

	static void RunFrame(CMasterNETHandler* This);
	void PacketReceived(sockaddr_in& from, byte* data, int length);

private:
	INT NS_SOCKET;
	SOCKET m_nClientSocket;
	SOCKET m_nServerSocket;
	HANDLE worker;
	bool workerRunning;
};

IMasterNETHandler* MasterNetHandler()
{
	static CMasterNETHandler instance;
	return &instance;
}

CMasterNETHandler::CMasterNETHandler() : m_nClientSocket(INVALID_SOCKET), m_nServerSocket(INVALID_SOCKET) {

	Msg("MasterNETHandler: startup\n");
	m_nClientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	m_nServerSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Client Socket can be used for broadcasting
	BOOL broadcast = TRUE;
	setsockopt(m_nClientSocket, SOL_SOCKET, SO_BROADCAST, (char*)&broadcast, sizeof(broadcast));

	u_long mode = 1;

	if (ioctlsocket(m_nClientSocket, FIONBIO, &mode) != 0)
		Warning("MasterNETHandler: Failed to set client socket non-blocking. WSA Error: %d\n", WSAGetLastError());

	if (ioctlsocket(m_nServerSocket, FIONBIO, &mode) != 0)
		Warning("MasterNETHandler: Failed to set server socket non-blocking. WSA Error: %d\n", WSAGetLastError());

	if (m_nClientSocket == INVALID_SOCKET || m_nServerSocket == INVALID_SOCKET) {
		Warning("MasterNETHandler %s socket creation failed.\n", (m_nClientSocket == INVALID_SOCKET) ? "client" : "server");
		return;
	}

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_nClientSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		Msg("MasterNETHandler client socket bind failed.\n");
		return;
	}

	// Cleanup address and allocate new for server socket

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(0);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(m_nServerSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		Msg("MasterNETHandler server socket bind failed.\n");
		return;
	}

	workerRunning = true;
	worker = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)RunFrame, this, 0, 0);
}

CMasterNETHandler::~CMasterNETHandler() {
	workerRunning = false;
	closesocket(m_nClientSocket);
	closesocket(m_nServerSocket);
	CloseHandle(worker);

	Msg("MasterNETHandler: shutdown\n");
}

void CMasterNETHandler::RunFrame(CMasterNETHandler* This) {
	Msg("MasterNETHandler listener started\n");

	char buffer[2048];

	while (This->workerRunning) {
		sockaddr_in sender{};
		int senderSize = sizeof(sender);

		master->RunFrame();
		//lanservers->RunFrame();

		int bytesClient = recvfrom(This->m_nClientSocket, buffer, sizeof(buffer), 0,
			(sockaddr*)&sender, &senderSize);
		if (bytesClient > 0 && bytesClient >= 4 && *(int*)buffer == -1)
		{
			This->PacketReceived(sender, (byte*)buffer, bytesClient);
		}

		int bytesServer = recvfrom(This->m_nServerSocket, buffer, sizeof(buffer), 0,
			(sockaddr*)&sender, &senderSize);
		if (bytesServer > 0 && bytesServer >= 4 && *(int*)buffer == -1)
		{
			This->PacketReceived(sender, (byte*)buffer, bytesServer);
		}
	}
}

void CMasterNETHandler::NET_SendPacket(int ns, const netadr_t& to, const byte* data, int length) {
	SOCKET* SendSocket = &m_nClientSocket;
	sockaddr addr {};
	to.ToSockadr(&addr);

	if (ns == NS_CLIENT)
		SendSocket = &m_nClientSocket;

	if (ns == NS_SERVER)
		SendSocket = &m_nServerSocket;

	NS_SOCKET = ns;

	int ret = sendto(*SendSocket, (const char*)data, length, 0, &addr, sizeof(addr));
	if (ret == SOCKET_ERROR)
		Warning("CMasterNETHandler: failed sending packet (socket %i, to %s, data %s, length %i), WSA Last Error %i\n", ns, to.ToString(), data, length, WSAGetLastError());
	//else
		//Msg("CMasterNETHandler: send packet socket %i, to %s, data %s, length %i\n", ns, to.ToString(), data, length);
}

void CMasterNETHandler::PacketReceived(sockaddr_in& from, byte* data, int length) {
	netpacket_t packet;
	byte* connectionless_data = data + 4;
	bf_read msg(connectionless_data, length - 4);

	packet.data = connectionless_data;

	packet.from.SetFromSockadr((sockaddr*)&from);

	packet.source = NS_SOCKET;

	packet.received = Plat_FloatTime();

	packet.message = msg;

	packet.size = length;

	packet.wiresize = length;

	packet.pNext = 0;

	//Msg("CMasterNETHandler: packet received from %s, data %s, length %i\n", packet.from.ToString(), data, length);

	// Check if this packet came from LAN
	//if (IsLANIP(packet.from.GetIP()))
	//	lanservers->ProcessConnectionlessPacket(&packet);
	//else
		master->ProcessConnectionlessPacket(&packet);

}