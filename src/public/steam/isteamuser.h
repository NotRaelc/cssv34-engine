//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: interface to user account information in Steam
//
//=============================================================================

#ifndef ISTEAMUSER_H
#define ISTEAMUSER_H
#ifdef _WIN32
#pragma once
#endif

#include "isteamclient.h"

// structure that contains client callback data
struct CallbackMsg_t
{
	HSteamUser m_hSteamUser;
	int m_iCallback;
	uint8 *m_pubParam;
	int m_cubParam;
};

// reference to a steam call, to filter results by
typedef int32 HSteamCall;

enum EAppUsageEvent
{
	k_EAppUsageEventGameLaunch = 1,
	k_EAppUsageEventGameLaunchTrial = 2,
	k_EAppUsageEventMedia = 3,
	k_EAppUsageEventPreloadStart = 4,
	k_EAppUsageEventPreloadFinish = 5,
	k_EAppUsageEventMarketingMessageView = 6,	// deprecated, do not use
	k_EAppUsageEventInGameAdViewed = 7,
	k_EAppUsageEventGameLaunchFreeWeekend = 8,
};

//-----------------------------------------------------------------------------
// Purpose: types of VAC bans
//-----------------------------------------------------------------------------
enum EVACBan
{
	k_EVACBanGoldsrc,
	k_EVACBanSource,
	k_EVACBanDayOfDefeatSource,
};

enum ELogonState
{
	k_ELogonStateNotLoggedOn = 0,
	k_ELogonStateLoggingOn = 1,
	k_ELogonStateLoggingOff = 2,
	k_ELogonStateLoggedOn = 3
};

enum ERegistrySubTree
{
	k_ERegistrySubTreeNews = 0,
	k_ERegistrySubTreeApps = 1,
	k_ERegistrySubTreeSubscriptions = 2,
	k_ERegistrySubTreeGameServers = 3,
	k_ERegistrySubTreeFriends = 4,
	k_ERegistrySubTreeSystem = 5,
};

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing and manipulating a steam account
//			associated with one client instance
//-----------------------------------------------------------------------------
class ISteamUser
{
public:

	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser() = 0;

	// steam account management functions
	virtual void LogOn( CSteamID steamID ) = 0;
	virtual void LogOff() = 0;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() = 0;

	virtual ELogonState GetLogonState() = 0;
	virtual bool BConnected() = 0;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() = 0;

	// returns true if this account is VAC banned from the specified ban set
	virtual bool IsVACBanned( EVACBan eVACBan ) = 0;

	// returns true if the user needs to see the newly-banned message from the specified ban set
	virtual bool RequireShowVACBannedMessage( EVACBan eVACBan ) = 0;

	// tells the server that the user has seen the 'you have been banned' dialog
	virtual void AcknowledgeVACBanning( EVACBan eVACBan ) = 0;

	// steam2 stuff
	virtual void SetSteam2Ticket( uint8 *pubTicket, int cubTicket ) = 0;
	virtual void AddServerNetAddress( uint32 unIP, uint16 unPort ) = 0;

	// email address setting
	virtual bool SetEmail( const char *pchEmail ) = 0;


	// persist per user data
	virtual bool SetRegistryString( ERegistrySubTree eRegistrySubTree, const char *pchKey, const char *pchValue ) = 0;
	virtual bool GetRegistryString( ERegistrySubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue ) = 0;
	virtual bool SetRegistryInt( ERegistrySubTree eRegistrySubTree, const char *pchKey, int iValue ) = 0;
	virtual bool GetRegistryInt( ERegistrySubTree eRegistrySubTree, const char *pchKey, int *piValue ) = 0;

	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	//
	// Parameters:
	// void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
	// int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// int nGameID - the ID of the current game.
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	// bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
	//
	// return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
	// The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.
	virtual int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, CGameID nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure ) = 0;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) = 0;

	// controls where chat messages go to - puts the caller on top of the stack of chat destinations
	virtual void SetSelfAsPrimaryChatDestination() = 0;
    	// returns true if the current caller is the one that should open new chat dialogs
	virtual bool IsPrimaryChatDestination() = 0;

	virtual void RequestLegacyCDKey( AppId_t iAppID ) = 0;

	virtual bool SendGuestPassByEmail( const char *pchEmailAccount, GID_t gidGuestPassID, bool bResending ) = 0;
	virtual bool SendGuestPassByAccountID( uint32 uAccountID, GID_t gidGuestPassID, bool bResending ) = 0;

	virtual bool AckGuestPass( const char *pchGuestPassCode ) = 0;
	virtual bool RedeemGuestPass( const char *pchGuestPassCode ) = 0;

	virtual uint32 GetGuestPassToGiveCount() = 0;
	virtual uint32 GetGuestPassToRedeemCount() = 0;
	virtual uint32 GetGuestPassLastUpdateTime() = 0;

	virtual bool GetGuestPassToGiveInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t* pnPackageID, RTime32* pRTime32Created, RTime32* pRTime32Expiration, RTime32* pRTime32Sent, RTime32* pRTime32Redeemed, char * pchRecipientAddress, int cRecipientAddressSize ) = 0;
	virtual bool GetGuestPassToRedeemInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t* pnPackageID, RTime32* pRTime32Created, RTime32* pRTime32Expiration, RTime32* pRTime32Sent, RTime32* pRTime32Redeemed ) = 0;
	virtual bool GetGuestPassToRedeemSenderAddress( uint32 nPassIndex, char* pchSenderAddress, int cSenderAddressSize ) = 0;
	virtual bool GetGuestPassToRedeemSenderName( uint32 nPassIndex, char* pchSenderName, int cSenderNameSize ) = 0;
	virtual void AcknowledgeMessageByGID( const char *pchMessageGID ) = 0;

	virtual bool SetLanguage( const char *pchLanguage ) = 0;

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, EAppUsageEvent eAppUsageEvent, const char *pchExtraInfo = "" ) = 0;

	virtual void SetAccountName( const char* pchAccountName ) = 0;
	virtual void SetPassword( const char* pchPassword ) = 0;

	virtual void SetAccountCreationTime( RTime32 rtime32Time ) = 0;
};

#define STEAMUSER_INTERFACE_VERSION "SteamUser005"


// callbacks


//-----------------------------------------------------------------------------
// Purpose: called when a logon attempt has succeeded
//-----------------------------------------------------------------------------
struct LogonSuccess_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 1 };
};


//-----------------------------------------------------------------------------
// Purpose: called when a logon attempt has failed
//-----------------------------------------------------------------------------
struct LogonFailure_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 2 };
	EResult m_eResult;
};


//-----------------------------------------------------------------------------
// Purpose: called when the user logs off
//-----------------------------------------------------------------------------
struct LoggedOff_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 3 };
	EResult m_eResult;
};


//-----------------------------------------------------------------------------
// Purpose: called when the client is trying to retry logon after being unintentionally logged off
//-----------------------------------------------------------------------------
struct BeginLogonRetry_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 4 };
};


//-----------------------------------------------------------------------------
// Purpose: called when the steam2 ticket has been set
//-----------------------------------------------------------------------------
struct Steam2TicketChanged_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 6 };
};


//-----------------------------------------------------------------------------
// Purpose: called when app news update is recieved
//-----------------------------------------------------------------------------
struct ClientAppNewsItemUpdate_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 10 };

	uint8 m_eNewsUpdateType;	// one of ENewsUpdateType
	uint32 m_uNewsID;			// unique news post ID
	uint32 m_uAppID;			// app ID this update applies to if it is of type k_EAppNews
};


//-----------------------------------------------------------------------------
// Purpose: steam news update
//-----------------------------------------------------------------------------
struct ClientSteamNewsItemUpdate_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 12 };

	uint8 m_eNewsUpdateType;	// one of ENewsUpdateType
	uint32 m_uNewsID;			// unique news post ID
	uint32 m_uHaveSubID;		// conditions to control if we display this update for type k_ESteamNews
	uint32 m_uNotHaveSubID;
	uint32 m_uHaveAppID;
	uint32 m_uNotHaveAppID;
	uint32 m_uHaveAppIDInstalled;
	uint32 m_uHavePlayedAppID;
};


//-----------------------------------------------------------------------------
// Purpose: connect to game server denied
//-----------------------------------------------------------------------------
struct ClientGameServerDeny_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 13 };

	uint32 m_uAppID;
	uint32 m_unGameServerIP;
	uint16 m_usGameServerPort;
	uint16 m_bSecure;
	uint32 m_uReason;
};


//-----------------------------------------------------------------------------
// Purpose: notifies the user that they are now the primary access point for chat messages
//-----------------------------------------------------------------------------
struct PrimaryChatDestinationSet_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 14 };
	uint8 m_bIsPrimary;
};


//-----------------------------------------------------------------------------
// Purpose: connect to game server denied
//-----------------------------------------------------------------------------
struct GSPolicyResponse_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 15 };
	uint8 m_bSecure;
};


//-----------------------------------------------------------------------------
// Purpose: steam cddb/bootstrapper update
//-----------------------------------------------------------------------------
struct ClientSteamNewsClientUpdate_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 16 };

	uint8 m_eNewsUpdateType;	// one of ENewsUpdateType
	uint8 m_bReloadCDDB; // if true there is a new CDDB available
	uint32 m_unCurrentBootstrapperVersion;
	uint32 m_unCurrentClientVersion;
};


//-----------------------------------------------------------------------------
// Purpose: called when the callback system for this client is in an error state (and has flushed pending callbacks)
//			When getting this message the client should disconnect from Steam, reset any stored Steam state and reconnect
//-----------------------------------------------------------------------------
struct CallbackPipeFailure_t
{
	enum { k_iCallback = k_iSteamUserCallbacks + 17 };
};


#endif // ISTEAMUSER_H
