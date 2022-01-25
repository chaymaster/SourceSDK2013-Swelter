//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef weapon_wrench2_H
#define weapon_wrench2_H

#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#ifdef HL2MP
#error weapon_wrench2.h must not be included in hl2mp. The windows compiler will use the wrong class elsewhere if it is.
#endif

#define	CROWBAR_RANGE	75.0f
#define	CROWBAR_REFIRE	0.4f

//-----------------------------------------------------------------------------
// CWeaponwrench2
//-----------------------------------------------------------------------------

class CWeaponwrench2 : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS( CWeaponwrench2, CBaseHLBludgeonWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponwrench2();

	float		GetRange( void )		{	return	CROWBAR_RANGE;	}
	float		GetFireRate( void )		{	return	CROWBAR_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );

	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		SecondaryAttack( void )	{	return;	}
	virtual bool	Deploy(void);

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};

#endif // weapon_wrench2_H
