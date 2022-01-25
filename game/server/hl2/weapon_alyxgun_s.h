//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef weapon_alyxgun_s_H
#define weapon_alyxgun_s_H

#include "basehlcombatweapon.h"
#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "IEffects.h"
#include "te_effect_dispatch.h"
#include "Sprite.h"
#include "SpriteTrail.h"
#include "beam_shared.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "decals.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponAlyxGun_s : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponAlyxGun_s, CHLSelectFireMachineGun );

	CWeaponAlyxGun_s();
	~CWeaponAlyxGun_s();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );

	virtual int		GetMinBurst( void ) { return 4; }
	virtual int		GetMaxBurst( void ) { return 7; }
	virtual float	GetMinRestTime( void );
	virtual float	GetMaxRestTime( void );
	virtual void	ItemPostFrame(void);
	void	PrimaryAttack(void);
	void	HoldIronsight(void);
	bool	Reload(void);
	bool	Deploy(void);


	virtual void Equip( CBaseCombatCharacter *pOwner );

	float	GetFireRate( void ) { return 0.1f; }
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack1Condition( float flDot, float flDist );
	int		WeaponRangeAttack2Condition( float flDot, float flDist );

	virtual const Vector& GetBulletSpread( void );

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, bool bUseWeaponAngles );

	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	//virtual void SetPickupTouch( void )
	//{
	//	// Alyx gun cannot be picked up
	//	SetTouch(NULL);
	//}

	float m_flTooCloseTimer;

	DECLARE_ACTTABLE();

};

#endif // weapon_alyxgun_s_H
