//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		357 - hand gun
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "gamerules.h"
#include "in_buttons.h"
#include "soundent.h"
#include "game.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "te_effect_dispatch.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CWeapon356
//-----------------------------------------------------------------------------

class CWeapon356 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeapon356, CBaseHLCombatWeapon );
public:

	CWeapon356( void );

	void	PrimaryAttack( void );
	void	HoldIronsight(void);
	virtual void	ItemPostFrame(void);
	virtual void	ItemBusyFrame(void);
	virtual bool			Reload(void);
	bool	Deploy(void);
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	AmmoLoadingCount(void); //ammo_loading_count
	bool	AmmoLoadingDo;  //ammo_loading_count
	float	AmmoLoadingTime; //ammo_loading_count


	float	WeaponAutoAimScale()	{ return 0.6f; }
	float	GetFireRate()			{ return 1.5f; }


	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( weapon_356, CWeapon356 );

PRECACHE_WEAPON_REGISTER( weapon_356 );

IMPLEMENT_SERVERCLASS_ST( CWeapon356, DT_Weapon356 )
END_SEND_TABLE()

BEGIN_DATADESC( CWeapon356 )
END_DATADESC()
#define BODYGROUP_AMMOLEFT 1 //ammo_loading_count
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon356::CWeapon356( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon356::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	switch( pEvent->event )
	{
		case EVENT_WEAPON_RELOAD:
			{
				CEffectData data;

				// Emit six spent shells
				for ( int i = 0; i < 6; i++ )
				{
					data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector( -4, 4 );
					data.m_vAngles = QAngle( 90, random->RandomInt( 0, 360 ), 0 );
					data.m_nEntIndex = entindex();

					DispatchEffect( "ShellEject", data );
				}

				break;
			}
	}
}
bool CWeapon356::Deploy(void)
{

	Msg("SDE_SMG!_deploy\n");
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	DisplaySDEHudHint();
	return BaseClass::Deploy();
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon356::PrimaryAttack( void )
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( !pPlayer )
	{
		return;
	}

	if ( m_iClip1 <= 0 )
	{
		if ( !m_bFireOnEmpty )
		{
			Reload();
		}
		else
		{
			WeaponSound( EMPTY );
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	
	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );

	WeaponSound( SINGLE ); //snd1
	pPlayer->DoMuzzleFlash();

	SendWeaponAnim( ACT_VM_PRIMARYATTACK );
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.1; //было 0,75
	m_flNextSecondaryAttack = gpGlobals->curtime + 1.1;

	m_iClip1--;

	Vector vecSrc		= pPlayer->Weapon_ShootPosition();
	Vector vecAiming	= pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	
	
	pPlayer->SetTimeBase(2);
	pPlayer->FireBullets( 1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0 );

	pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 0.5 );

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt( -2, 2 );
	angles.y += random->RandomInt( -2, 2 );
	angles.z = 0;

	pPlayer->SnapEyeAngles( angles );

	pPlayer->ViewPunch( QAngle( -8, random->RandomFloat( -2, 2 ), 0 ) );

	CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner() );

	if ( !m_iClip1 && pPlayer->GetAmmoCount( m_iPrimaryAmmoType ) <= 0 )
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate( "!HEV_AMO0", FALSE, 0 ); 
	}
}

void CWeapon356::HoldIronsight(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer->m_afButtonPressed & IN_IRONSIGHT)
	{
		EnableIronsights();
		pPlayer->ShowCrosshair(false);
	}
	if (pPlayer->m_afButtonReleased & IN_IRONSIGHT)
	{
		DisableIronsights();
		pPlayer->ShowCrosshair(true);
	}
}

void CWeapon356::ItemPostFrame(void)
{
	// Allow  Ironsight
	// Ironsight if not reloading
	if (!m_bInReload)
		HoldIronsight();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	BaseClass::ItemPostFrame();
}

void CWeapon356::ItemBusyFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	if ((AmmoLoadingDo) && (gpGlobals->curtime > AmmoLoadingTime)) //ammo_loading_count
	{
		DevMsg("SDE: revolver test 5\n");
		AmmoLoadingCount();
	}

	BaseClass::ItemBusyFrame();
}

bool CWeapon356::Reload(void)	//ammo_loading_count
{
	DevMsg("SDE: revolver test 1\n");
	
	bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
	if (fRet)
	{
		AmmoLoadingTime = (gpGlobals->curtime + 1.7f);
		AmmoLoadingDo = true;
		DevMsg("SDE: revolver test 2\n");
	}
	return fRet;
}

void CWeapon356::AmmoLoadingCount(void) //ammo_loading_count
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	//m_iClip1 в оружии
	//m_iPrimaryAmmoType в кармане

	DevMsg("SDE: revolver test 3\n");
	switch (pOwner->GetAmmoCount(m_iPrimaryAmmoType + m_iClip1))
	{
	case 1:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 5);
		break;
	case 2:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 4);
		break;
	case 3:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 3);
		break;
	case 4:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 2);
		break;
	case 5:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 1);
		break;
	default:
		pViewModel->SetBodygroup(BODYGROUP_AMMOLEFT, 0);

		AmmoLoadingDo = false;
	}

}