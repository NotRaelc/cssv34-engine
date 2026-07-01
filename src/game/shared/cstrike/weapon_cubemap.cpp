//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#if defined( CLIENT_DLL )
#define CWeaponCubemap C_WeaponCubemap
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CWeaponCubemap : public CBaseCombatWeapon
{
public:
    DECLARE_CLASS(CWeaponCubemap, CBaseCombatWeapon);
    DECLARE_NETWORKCLASS();
    DECLARE_PREDICTABLE();

    virtual void Precache(void);
    virtual bool HasAnyAmmo(void) { return true; }
    virtual void Spawn(void);
};

IMPLEMENT_NETWORKCLASS_ALIASED(WeaponCubemap, DT_WeaponCubemap)

BEGIN_NETWORK_TABLE(CWeaponCubemap, DT_WeaponCubemap)
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA(CWeaponCubemap)
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS(weapon_cubemap, CWeaponCubemap);
PRECACHE_WEAPON_REGISTER(weapon_cubemap);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponCubemap::Precache(void)
{
    BaseClass::Precache();
}

void CWeaponCubemap::Spawn(void)
{
    BaseClass::Spawn();

    //Hack to fix the cubemap weapon not being held by the player.
    //Problem is the model has huge bounds so the new pickup code that checks if the player can see the model fails cause half the entity's bounds are inside the ground.
    //Since this is just a dev tool I made this quick hack so level designers can use it again asap. - Adrian
    SetCollisionBounds(Vector(-16, -16, -16), Vector(16, 16, 16));
}