//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef ISTEAMBILLING_H
#define ISTEAMBILLING_H
#ifdef _WIN32
#pragma once
#endif

#include "isteamclient.h"

// Flags for licenses - BITS
typedef enum ELicenseFlags
{
	k_ELicenseFlagNone = 0,
	k_ELicenseFlagRenew = 0x01,				// Renew this license next period
	k_ELicenseFlagRenewalFailed = 0x02,		// Auto-renew failed
	k_ELicenseFlagPending = 0x04,			// Purchase or renewal is pending
	k_ELicenseFlagExpired = 0x08,			// Regular expiration (no renewal attempted)
	k_ELicenseFlagCancelledByUser = 0x10,	// Cancelled by the user
	k_ELicenseFlagCancelledByAdmin = 0x20,	// Cancelled by customer support
	k_ELicenseFlagLowViolence = 0x40,
	k_ELicenseFlagImportedFromSteam2 = 0x80,
} ELicenseFlags;

// Payment methods for purchases
typedef enum EPaymentMethod
{
	k_EPaymentMethodNone = 0,
	k_EPaymentMethodActivationCode = 1,		
	k_EPaymentMethodCreditCard = 2,
	k_EPaymentMethodGiropay = 3,
	k_EPaymentMethodPayPal = 4,
	k_EPaymentMethodIdeal = 5,
	k_EPaymentMethodPaySafeCard = 6,
	k_EPaymentMethodSofort = 7,
	k_EPaymentMethodGuestPass = 8,
	k_EPaymentMethodWebMoney = 9,
	k_EPaymentMethodMoneyBookers = 10,
	k_EPaymentMethodAliPay = 11,
	k_EPaymentMethodYandex = 12,
	k_EPaymentMethodKiosk = 13,
	k_EPaymentMethodQIWI = 14,
	k_EPaymentMethodGameStop = 15,
	k_EPaymentMethodHardwarePromo = 16,
	k_EPaymentMethodMopay = 17,
	k_EPaymentMethodBoletoBancario = 18,
	k_EPaymentMethodBoaCompraGold = 19,
	k_EPaymentMethodBancoDoBrasilOnline = 20,
	k_EPaymentMethodItauOnline = 21,
	k_EPaymentMethodBradescoOnline = 22,
	k_EPaymentMethodPagseguro = 23,
	k_EPaymentMethodVisaBrazil = 24,
	k_EPaymentMethodAmexBrazil = 25,
	k_EPaymentMethodAura = 26,
	k_EPaymentMethodHipercard = 27,
	k_EPaymentMethodMastercardBrazil = 28,
	k_EPaymentMethodDinerSCardBrazil = 29,
	k_EPaymentMethodAuthorizedDevice = 30,
	k_EPaymentMethodMOLPoints = 31,
	k_EPaymentMethodClickAndBuy = 32,
	k_EPaymentMethodBeeline = 33,
	k_EPaymentMethodKonbini = 34,
	k_EPaymentMethodEClubPoints = 35,
	k_EPaymentMethodCreditCardJapan = 36,
	k_EPaymentMethodBankTransferJapan = 37,
	k_EPaymentMethodPayEasyJapan = 38,

	k_EPaymentMethodSteamPressMaster = 130,

	k_EPaymentMethodAutoGrant = 64,
	k_EPaymentMethodWallet = 128,
	k_EPaymentMethodValve = 129,
	k_EPaymentMethodOEMTicket = 256,
	k_EPaymentMethodSplit = 512,
	k_EPaymentMethodComplimentary = 1024,
} EPaymentMethod;

typedef enum EPurchaseResultDetail
{
	k_EPurchaseResultNoDetail = 0,
	k_EPurchaseResultAVSFailure = 1,
	k_EPurchaseResultInsufficientFunds = 2,
	k_EPurchaseResultContactSupport = 3,
	k_EPurchaseResultTimeout = 4,

	// these are mainly used for testing
	k_EPurchaseResultInvalidPackage = 5,
	k_EPurchaseResultInvalidPaymentMethod = 6,
	k_EPurchaseResultInvalidData = 7,
	k_EPurchaseResultOthersInProgress = 8,
	k_EPurchaseResultAlreadyPurchased = 9,
	k_EPurchaseResultWrongPrice = 10,

	k_EPurchaseResultFraudCheckFailed = 11,
	k_EPurchaseResultCancelledByUser = 12,
	k_EPurchaseResultRestrictedCountry = 13,
	k_EPurchaseResultBadActivationCode = 14,
	k_EPurchaseResultDuplicateActivationCode = 15,
	k_EPurchaseResultUseOtherPaymentMethod = 16,
	k_EPurchaseResultUseOtherFundingSource = 17,
	k_EPurchaseResultInvalidShippingAddress = 18,
	k_EPurchaseResultRegionNotSupported = 19,
	k_EPurchaseResultAcctIsBlocked = 20,
	k_EPurchaseResultAcctNotVerified = 21,
	k_EPurchaseResultInvalidAccount = 22,
	k_EPurchaseResultStoreBillingCountryMismatch = 23,
	k_EPurchaseResultDoesNotOwnRequiredApp = 24,
	k_EPurchaseResultCanceledByNewTransaction = 25,
	k_EPurchaseResultForceCanceledPending = 26,
	k_EPurchaseResultFailCurrencyTransProvider = 27,
	k_EPurchaseResultFailedCyberCafe = 28,
	k_EPurchaseResultNeedsPreApproval = 29,
	k_EPurchaseResultPreApprovalDenied = 30,
	k_EPurchaseResultWalletCurrencyMismatch = 31,
	k_EPurchaseResultEmailNotValidated = 32,
	k_EPurchaseResultExpiredCard = 33,
	k_EPurchaseResultTransactionExpired = 34,
	k_EPurchaseResultWouldExceedMaxWallet = 35,
	k_EPurchaseResultMustLoginPS3AppForPurchase = 36,
	k_EPurchaseResultCannotShipToPOBox = 37,
} EPurchaseResultDetail;

typedef enum EPurchaseStatus
{
	k_EPurchasePending = 0,
	k_EPurchaseSucceeded = 1,
	k_EPurchaseFailed = 2,
	k_EPurchaseRefunded = 3,
	k_EPurchaseInit = 4,
	k_EPurchaseChargedback = 5,
	k_EPurchaseRevoked = 6,
	k_EPurchaseInDispute = 7,
	k_EPurchasePartialRefund = 8,
	k_EPurchaseRefundToWallet = 9,
} EPurchaseStatus;

typedef enum ECreditCardType
{
	k_ECreditCardTypeUnknown = 0,
	k_ECreditCardTypeVisa = 1,
	k_ECreditCardTypeMaster = 2,
	k_ECreditCardTypeAmericanExpress = 3,
	k_ECreditCardTypeDiscover = 4,
	k_ECreditCardTypeDinersClub = 5,
	k_ECreditCardTypeJCB = 6,
	k_ECreditCardTypeCarteBleue = 7,
	k_ECreditCardTypeDankort = 8,
	k_ECreditCardTypeMaestro = 9,
	k_ECreditCardTypeSolo = 10,
	k_ECreditCardTypeLaser = 11,
} ECreditCardType;

enum ELicenseType
{
	k_ENoLicense = 0,
	k_ESinglePurchase = 1,
	k_ESinglePurchaseLimitedUse = 2,
	k_ERecurringCharge = 3,
	k_ERecurringChargeLimitedUse = 4,
	k_ERecurringChargeLimitedUseWithOverages = 5,
};


#pragma pack( push, 8 )
//-----------------------------------------------------------------------------
// Purpose: called when this client has received a finalprice message from a Billing
//-----------------------------------------------------------------------------
struct FinalPriceMsg_t
{
		enum { k_iCallback = k_iSteamBillingCallbacks + 1 };

		uint32 m_bSuccess;
		uint32 m_nBaseCost;
		uint32 m_nTotalDiscount;
		uint32 m_nTax;
		uint32 m_nShippingCost;
};

struct PurchaseMsg_t
{
		enum { k_iCallback = k_iSteamBillingCallbacks + 2 };

		uint32 m_bSuccess;
		int32 m_EPurchaseResultDetail;			// Detailed result information
};

// Sent in response to PurchaseWithActivationCode
struct PurchaseResponse_t
{
	enum { k_iCallback = k_iSteamBillingCallbacks + 4 };
	
	EResult m_EResult;
	int32 m_EPurchaseResultDetail;
	int32 m_iReceiptIndex;
};

struct CancelLicenseMsg_t
{
	enum { k_iCallback = k_iSteamBillingCallbacks + 9 };

	enum EResult m_EResult;
};

struct RequestFreeLicenseResponse_t
{
	enum { k_iCallback = k_iSteamBillingCallbacks + 12 };

	EResult m_EResult;
	AppId_t m_uAppId;
};

struct OEMTicketActivationResponse_t
{
	enum { k_iCallback = k_iSteamBillingCallbacks + 14 };

	EResult m_EResult;
	EPurchaseResultDetail m_EPurchaseResultDetail;
	PackageId_t m_nPackageID;
	int m_iReceiptIndex;
};
#pragma pack( pop )

//-----------------------------------------------------------------------------
// Purpose: interface to billing
//-----------------------------------------------------------------------------
class ISteamBilling
{
public:

	virtual bool InitCreditCardPurchase( PackageId_t nPackageID, uint32 nCardIndex, bool bStoreCardInfo ) = 0;
	virtual bool InitPayPalPurchase( PackageId_t nPackageID ) = 0;

	virtual bool GetActivationCodeInfo( const char *pchActivationCode ) = 0;
	virtual bool PurchaseWithActivationCode( const char *pchActivationCode ) = 0;

	virtual bool GetFinalPrice() = 0;

	virtual bool CancelPurchase() = 0;
	virtual bool CompletePurchase() = 0;

	virtual bool UpdateCardInfo( uint32 nCardIndex ) = 0;

	virtual bool DeleteCard( uint32 ) = 0;

	virtual bool GetCardList() = 0;

	virtual bool Obsolete_GetLicenses() = 0;

	virtual bool CancelLicense( PackageId_t nPackageID, int32 nCancelReason ) = 0;

	virtual bool GetPurchaseReceipts( bool bUnacknowledgedOnly ) = 0;

	// Sets the billing address in the ISteamBilling object for use by other ISteamBilling functions (not stored on server)
	virtual bool SetBillingAddress( uint32 nCardIndex, const char *pchFirstName, const char *pchLastName, const char *pchAddress1, const char *pchAddress2, const char *pchCity, const char *pchPostcode, const char *pchState, const char *pchCountry, const char *pchPhone ) = 0;

	// Gets any previous set billing address in the ISteamBilling object (not stored on server)
	virtual bool GetBillingAddress( uint32 nCardIndex, char *pchFirstName, char *pchLastName, char *pchAddress1, char *pchAddress2, char *pchCity, char *pchPostcode, char *pchState, char *pchCountry, char *pchPhone ) = 0;

	// Sets the billing address in the ISteamBilling object for use by other ISteamBilling functions (not stored on server)
	virtual bool SetShippingAddress( const char *pchFirstName, const char *pchLastName, const char *pchAddress1, const char *pchAddress2, const char *pchCity, const char *pchPostcode, const char *pchState, const char *pchCountry, const char *pchPhone ) = 0;
	// Gets any previous set billing address in the ISteamBilling object (not stored on server)
	virtual bool GetShippingAddress( char *pchFirstName, char *pchLastName, char *pchAddress1, char *pchAddress2, char *pchCity, char *pchPostcode, char *pchState, char *pchCountry, char *pchPhone ) = 0;

	// Sets the credit card info in the ISteamBilling object for use by other ISteamBilling functions  (may eventually also be stored on server)
	virtual bool SetCardInfo( uint32 nCardIndex, ECreditCardType eCreditCardType, const char *pchCardNumber, const char *pchCardHolderFirstName, const char *pchCardHolderLastName, const char *pchCardExpYear, const char *pchCardExpMonth, const char *pchCardCVV2 ) = 0;
	// Gets any credit card info in the ISteamBilling object (not stored on server)
	virtual bool GetCardInfo( uint32 nCardIndex, ECreditCardType *eCreditCardType, char *pchCardNumber, char *pchCardHolderFirstName, char *pchCardHolderLastName, char *pchCardExpYear, char *pchCardExpMonth, char *pchCardCVV2 ) = 0;

	virtual PackageId_t GetLicensePackageID( uint32 nLicenseIndex ) = 0;
	virtual RTime32 GetLicenseTimeCreated( uint32 nLicenseIndex ) = 0;
	virtual RTime32 GetLicenseTimeNextProcess( uint32 nLicenseIndex ) = 0;
	virtual int32 GetLicenseMinuteLimit( uint32 nLicenseIndex ) = 0;
	virtual int32 GetLicenseMinutesUsed( uint32 nLicenseIndex ) = 0;
	virtual EPaymentMethod GetLicensePaymentMethod( uint32 nLicenseIndex ) = 0;
	virtual ELicenseFlags GetLicenseFlags( uint32 nLicenseIndex ) = 0;
	virtual const char *GetLicensePurchaseCountryCode( uint32 nLicenseIndex ) = 0;

	virtual PackageId_t GetReceiptPackageID( uint32 nReceiptIndex ) = 0;
	virtual EPurchaseStatus GetReceiptStatus( uint32 nReceiptIndex ) = 0;
	virtual EPurchaseResultDetail GetReceiptResultDetail( uint32 nReceiptIndex ) = 0;
	virtual RTime32 GetReceiptTransTime( uint32 nReceiptIndex ) = 0;
	virtual uint64 GetReceiptTransID( uint32 nReceiptIndex ) = 0;
	virtual EPaymentMethod GetReceiptPaymentMethod( uint32 nReceiptIndex ) = 0;
	virtual uint32 GetReceiptBaseCost( uint32 nReceiptIndex ) = 0;
	virtual uint32 GetReceiptTotalDiscount( uint32 nReceiptIndex ) = 0;
	virtual uint32 GetReceiptTax( uint32 nReceiptIndex ) = 0;
	virtual uint32 GetReceiptShipping( uint32 nReceiptIndex ) = 0;
	virtual const char *GetReceiptCountryCode( uint32 nReceiptIndex ) = 0;

	virtual uint32 GetNumLicenses() = 0;
	virtual uint32 GetNumReceipts() = 0;

	virtual bool PurchaseWithMachineID( PackageId_t nPackageID, const char *pchCustomData ) = 0;

	virtual bool InitClickAndBuyPurchase( PackageId_t nPackageID, int64 nAccountNum, const char *pchState, const char *pchCountryCode ) = 0;

	virtual bool GetPreviousClickAndBuyAccount( int64 *pnAccountNum, char *pchState, char *pchCountryCode ) = 0;

};


#define STEAMBILLING_INTERFACE_VERSION "SteamBilling002"

#endif // ISTEAMBILLING_H

