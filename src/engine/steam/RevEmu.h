#pragma once

#include "RevSpoofer.h"
#include "Encryption\CRijndael.h"
#include "Encryption\SHA.h"
#include <Windows.h>

#define REV_DO_ENCRYPT_HWID true
static const int ticket_size = 178; // RevCrew SC2009

static void CreateRandomString(char *pszDest, int nLength)
{
	static const char c_szAlphaNum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	for (int i = 0; i < nLength; ++i)
		pszDest[i] = c_szAlphaNum[rand() % (sizeof(c_szAlphaNum) - 1)];

	pszDest[nLength] = '\0';
}


int GenerateRevEmu(void* pDest, int nSteamID, int Gen = 4)
{
	char hwid[64];

	CreateRandomString(hwid, 32);
	if (!RevSpoofer::Spoof(hwid, nSteamID))
		return 0;

	auto pTicket = (int*)pDest;
	auto pbTicket = (byte*)pDest;

	auto revHash = RevSpoofer::Hash(hwid);
	
	if (Gen == 3)
		pTicket[0] = 'J';          // +0
	else 
		pTicket[0] = 'S';          // +0

	pTicket[1] = revHash;      // +4
	pTicket[2] = 'rev';        // +8
	pTicket[3] = 0;            // +12
	pTicket[4] = revHash << 1; // +16
	pTicket[5] = 0x01100001;   // +20

	if (Gen == 4)
	{
		/* Encrypt HWID with AESKeyRand key and save it in the ticket. */
		static const char AESKeyRand[] = "0123456789ABCDEFGHIJKLMNOPQRSTUV";
		char AESHashRand[32];
		auto AESRand = CRijndael();
		AESRand.MakeKey(AESKeyRand, CRijndael::sm_chain0, 32, 32);
		AESRand.EncryptBlock(hwid, AESHashRand);
		memcpy(&pbTicket[24], AESHashRand, 32);

		/* Encrypt AESKeyRand with AESKeyRev key and save it in the ticket.
		 * AESKeyRev key is identical to the key in dproto/reunion. */
		static const char AESKeyRev[] = "_YOU_SERIOUSLY_NEED_TO_GET_LAID_";
		char AESHashRev[32];
		auto AESRev = CRijndael();
		AESRev.MakeKey(AESKeyRev, CRijndael::sm_chain0, 32, 32);
		AESRev.EncryptBlock(AESKeyRand, AESHashRev);
		memcpy(&pbTicket[56], AESHashRev, 32);

		/* Perform HWID hashing and save hash to the ticket. */
		char SHAHash[32];
		auto sha = CSHA(CSHA::SHA256);
		sha.AddData(hwid, 32);
		sha.FinalDigest(SHAHash);
		memcpy(&pbTicket[88], SHAHash, 32);
	}
	else
	{
		// 3rd Generation does not require hwid encryption
		strcpy((char*)&pTicket[6], hwid); // +24, string for hash
	}

	if (Gen == 3)
		return 164;

	// stubs for Gen 2 and 1 (they both based on SteamEmu)
	if (Gen == 2)
		return 10; // RevEmu 9.6

	if (Gen == 1)
		return 768; // SteamEmu

	return ticket_size;
}