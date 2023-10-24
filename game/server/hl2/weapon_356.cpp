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

// extern ConVar	sde_drop_mag;

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
	bool	Deploy(void);
	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	float	WeaponAutoAimScale()	{ return 0.6f; }
	float	GetFireRate()			{ return 1.5f; }


	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
protected:
	int m_iNumberOfRoundsInDrum;
};

LINK_ENTITY_TO_CLASS( weapon_356, CWeapon356 );

PRECACHE_WEAPON_REGISTER( weapon_356 );

IMPLEMENT_SERVERCLASS_ST( CWeapon356, DT_Weapon356 )
END_SEND_TABLE()

BEGIN_DATADESC( CWeapon356 )

DEFINE_FIELD (m_iNumberOfRoundsInDrum, FIELD_INTEGER),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon356::CWeapon356( void )
{
	m_bReloadsSingly	= false;
	m_bFiresUnderwater	= false;
	m_iNumberOfRoundsInDrum = 6; //our revolver is found loaded, we're not EZ2 - HEVcrab
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon356::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBaseCombatCharacter *BCCOwner = GetOwner();
	CBasePlayer *pOwner = ToBasePlayer( BCCOwner );

	switch( pEvent->event )
	{
		case EVENT_WEAPON_RELOAD:
			{
				//regardless of sde_drop_mag value, eject only spent casings 
				//if (sde_drop_mag.GetInt())
				//{
					CEffectData data;
					// Eject (how many rounds were loaded previous time minus remaining ammo) spent casings (even for realistic mag drop, let the player keep rounds remaining in the drum) 
					for (int i = 0; i < m_iNumberOfRoundsInDrum - m_iClip1; i++)
					{
						data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector(-4, 4);
						data.m_vAngles = QAngle(90, random->RandomInt(0, 360), 0);
						data.m_nEntIndex = entindex();

						DispatchEffect("ShellEject", data);
					}
				        m_iNumberOfRoundsInDrum = MIN(GetWpnData().iMaxClip1, m_iClip1 + BCCOwner->GetAmmoCount(m_iPrimaryAmmoType));
				//}
				break;
			}
	}
}
bool CWeapon356::Deploy(void)
{

	DevMsg("SDE_SMG!_deploy\n");
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

	m_flNextPrimaryAttack = gpGlobals->curtime + 1.1; //áûëî 0,75
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


	BaseClass::ItemPostFrame();
}
