//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "quakedef.h"
#include "winlite.h"
#include "tier1/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//char *date = "Nov 07 1998"; // "Oct 24 1996";
char *date = __DATE__ ;

char *mon[12] = 
{ "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
char mond[12] = 
{ 31,    28,    31,    30,    31,    30,    31,    31,    30,    31,    30,    31 };

class CBuildNumber
{
public:
	CBuildNumber( void )
	{
		ComputeBuildNumber();
	}
	
	// returns days since Nov 07 1998
	int	GetBuildNumber( void ) 
	{
		return m_nBuildNumber;
	}

private:
	void		ComputeBuildNumber( void )
	{
		int m = 0; 
		int d = 0;
		int y = 0;

		for (m = 0; m < 11; m++)
		{
			if (Q_strncasecmp( &date[0], mon[m], 3 ) == 0)
				break;
			d += mond[m];
		}

		d += atoi( &date[4] ) - 1;

		y = atoi( &date[7] ) - 1900;

		m_nBuildNumber = d + (int)((y - 1) * 365.25);

		if (((y % 4) == 0) && m > 1)
		{
			m_nBuildNumber += 1;
		}

		//m_nBuildNumber -= 34995; // Oct 24 1996
		m_nBuildNumber -= 35739;  // Nov 7 1998 (HL1 Gold Date)
	}

	int			m_nBuildNumber;
};

// Singleton
static CBuildNumber g_BuildNumber;

//-----------------------------------------------------------------------------
// Purpose: Old build number value implementation 
// Output : int
//-----------------------------------------------------------------------------
int build_number( void )
{
	return g_BuildNumber.GetBuildNumber();
}

///////////////////////////////////////////////////////////////////////////////
///////// New style build number implementation using timestamp. //////////////
///////////////////////////////////////////////////////////////////////////////

extern "C" IMAGE_DOS_HEADER __ImageBase;

//-----------------------------------------------------------------------------
// Purpose: Get timestamp from nt header
// Output : unsigned int
//-----------------------------------------------------------------------------
uint32 build_timestamp()
{
	auto base = (BYTE*)&__ImageBase;

	auto dos = (PIMAGE_DOS_HEADER)base;
	auto nt = (PIMAGE_NT_HEADERS)(base + dos->e_lfanew);

	uint32 timestamp = nt->FileHeader.TimeDateStamp;

	return timestamp;
}

//-----------------------------------------------------------------------------
// Purpose: Convert timestamp(build number) to hex string.
// Output : char*
//-----------------------------------------------------------------------------
char* build_hex()
{
	char buffer[256];

	uint32 timestamp = build_timestamp();

	sprintf(buffer, "%x", timestamp);

	return buffer;
}