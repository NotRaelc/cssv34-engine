//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#ifndef SMOKE_TRAIL_H
#define SMOKE_TRAIL_H

#include "baseparticleentity.h"

//==================================================
// SmokeTrail
//==================================================

class SmokeTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(SmokeTrail, CBaseParticleEntity);
	// DECLARE_SERVERCLASS();

	SmokeTrail();
	virtual bool KeyValue(const char* szKeyName, const char* szValue);
	void					SetEmit(bool bVal);
	void					FollowEntity(CBaseEntity* pEntity, const char* pAttachmentName = NULL);
	static	SmokeTrail* CreateSmokeTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	Vector  m_StartColor;			// Fade between these colors.
	Vector  m_EndColor;
	float	m_Opacity;

	float	m_SpawnRate;			// How many particles per second.
	float	m_ParticleLifetime;		// How long do the particles live?
	float	m_StopEmitTime;			// When do I stop emitting particles?
	float	m_MinSpeed;				// Speed range.
	float	m_MaxSpeed;
	float	m_StartSize;			// Size ramp.
	float	m_EndSize;
	float	m_SpawnRadius;
	float	m_MinDirectedSpeed;		// Speed range.
	float	m_MaxDirectedSpeed;
	bool	m_bEmit;

	int		m_nAttachment;
};

//==================================================
// RocketTrail
//==================================================

class RocketTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(RocketTrail, CBaseParticleEntity);
	// DECLARE_SERVERCLASS();

	RocketTrail();
	void					SetEmit(bool bVal);
	void					FollowEntity(CBaseEntity* pEntity, const char* pAttachmentName = NULL);
	static RocketTrail*		CreateRocketTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	Vector  m_StartColor;			// Fade between these colors.
	Vector  m_EndColor;
	float	m_Opacity;

	float	m_SpawnRate;			// How many particles per second.
	float	m_ParticleLifetime;		// How long do the particles live?
	float	m_StopEmitTime;			// When do I stop emitting particles?
	float	m_MinSpeed;				// Speed range.
	float	m_MaxSpeed;
	float	m_StartSize;			// Size ramp.
	float	m_EndSize;
	float	m_SpawnRadius;

	bool	m_bEmit;

	int		m_nAttachment;

	bool	m_bDamaged;

	float	m_flFlareScale;			// Size of the flare
};

//==================================================
// SporeTrail
//==================================================

class SporeTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(SporeTrail, CBaseParticleEntity);
	// DECLARE_SERVERCLASS();

	SporeTrail(void);

	static SporeTrail* CreateSporeTrail();

	//Data members
public:

	Vector  m_vecEndColor;

	float	m_flSpawnRate;
	float	m_flParticleLifetime;
	float	m_flStartSize;
	float	m_flEndSize;
	float	m_flSpawnRadius;

	bool	m_bEmit;
};

//==================================================
// SporeExplosion
//==================================================

class SporeExplosion : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( SporeExplosion, CBaseParticleEntity );
	// DECLARE_SERVERCLASS();

	SporeExplosion( void );
	void Spawn( void );

	static SporeExplosion*		CreateSporeExplosion();

	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );

//Data members
public:

	bool m_bDisabled;

	float	m_flSpawnRate;
	float	m_flParticleLifetime;
	float	m_flStartSize;
	float	m_flEndSize;
	float	m_flSpawnRadius;

	bool	m_bEmit;
	bool	m_bDontRemove;
};

//==================================================
// CFireTrail
//==================================================

class CFireTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CFireTrail, CBaseParticleEntity);
	// DECLARE_SERVERCLASS();

	static CFireTrail* CreateFireTrail(void);
	void				FollowEntity(CBaseEntity* pEntity, const char* pAttachmentName);
	void				Precache(void);

	int   m_nAttachment;
	float m_flLifetime;
};

//==================================================
// DustTrail
//==================================================

class DustTrail : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(DustTrail, CBaseParticleEntity);
	// DECLARE_SERVERCLASS();

	DustTrail();
	virtual bool KeyValue(const char* szKeyName, const char* szValue);
	void					SetEmit(bool bVal);
	static	DustTrail* CreateDustTrail();

public:
	// Effect parameters. These will assume default values but you can change them.
	Vector  m_Color;
	float	m_Opacity;

	float	m_SpawnRate;			// How many particles per second.
	float	m_ParticleLifetime;		// How long do the particles live?
	float	m_StopEmitTime;			// When do I stop emitting particles?
	float	m_MinSpeed;				// Speed range.
	float	m_MaxSpeed;
	float	m_StartSize;			// Size ramp.
	float	m_EndSize;
	float	m_SpawnRadius;
	float	m_MinDirectedSpeed;		// Speed range.
	float	m_MaxDirectedSpeed;
	bool	m_bEmit;

	int		m_nAttachment;
};


#endif
