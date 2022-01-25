//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_frag.h"
#include "npcevent.h"
#include "engine/IEngineSound.h"
#include "items.h"
#include "in_buttons.h"
#include "soundent.h"
#include "gamestats.h"
#include "grenade_ar2.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define GRENADE_TIMER	3.0f //Seconds было 3,0

#define GRENADE_PAUSED_NO			0
#define GRENADE_PAUSED_PRIMARY		1
#define GRENADE_PAUSED_SECONDARY	2

#define GRENADE_RADIUS	4.0f // inches

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CWeaponHandgren: public CBaseHLCombatWeapon
{
	DECLARE_CLASS( CWeaponHandgren, CBaseHLCombatWeapon );
public:
	DECLARE_SERVERCLASS();

public:
	CWeaponHandgren();

	void	Precache( void );

	void	Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void	PrimaryAttack( void );
	void	SecondaryAttack( void );
	void	DecrementAmmo( CBaseCombatCharacter *pOwner );
	void	ItemPostFrame( void );

	bool	Deploy( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );

	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	
	bool	Reload( void );

	bool	ShouldDisplayHUDHint() { return true; }

private:
	void	ThrowGrenade( CBasePlayer *pPlayer );
	void	RollGrenade( CBasePlayer *pPlayer );
	void	LobGrenade( CBasePlayer *pPlayer );
	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
	void	CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc );

	bool	m_bRedraw;	//Draw the weapon again after throwing a grenade
	
	int		m_AttackPaused;
	bool	m_fDrawbackFinished;


	DECLARE_ACTTABLE();

	DECLARE_DATADESC();
};


BEGIN_DATADESC( CWeaponHandgren )
	DEFINE_FIELD( m_bRedraw, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_AttackPaused, FIELD_INTEGER ),
	DEFINE_FIELD( m_fDrawbackFinished, FIELD_BOOLEAN ),
END_DATADESC()

acttable_t	CWeaponHandgren::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeaponHandgren);

IMPLEMENT_SERVERCLASS_ST(CWeaponHandgren, DT_WeaponHandgren)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_handgren, CWeaponHandgren );
PRECACHE_WEAPON_REGISTER(weapon_handgren);



CWeaponHandgren::CWeaponHandgren() :
	CBaseHLCombatWeapon(),
	m_bRedraw( false )
{
	NULL;
	m_bIsIronsighted = false;
	m_bCanIronsighted = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHandgren::Precache( void )
{
	BaseClass::Precache();

	UTIL_PrecacheOther( "grenade_ar2" );

	PrecacheScriptSound( "WeaponFrag.Throw" );
	PrecacheScriptSound( "WeaponFrag.Roll" );
}
extern ConVar sk_plr_dmg_smg1_grenade;
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponHandgren::Deploy( void )
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	DisplaySDEHudHint();
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHandgren::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;
	m_fDrawbackFinished = false;

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponHandgren::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	bool fThrewGrenade = false;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
			m_fDrawbackFinished = true;
			break;

		case EVENT_WEAPON_THROW:
			ThrowGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW2:
			RollGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		case EVENT_WEAPON_THROW3:
			LobGrenade( pOwner );
			DecrementAmmo( pOwner );
			fThrewGrenade = true;
			break;

		default:
			BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
			break;
	}

#define RETHROW_DELAY	0.5
	if( fThrewGrenade )
	{
		m_flNextPrimaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flNextSecondaryAttack	= gpGlobals->curtime + RETHROW_DELAY;
		m_flTimeWeaponIdle = FLT_MAX; //NOTE: This is set once the animation has finished up!

		// Make a sound designed to scare snipers back into their holes!
		CBaseCombatCharacter *pOwner = GetOwner();

		if( pOwner )
		{
			Vector vecSrc = pOwner->Weapon_ShootPosition();
			Vector	vecDir;

			AngleVectors( pOwner->EyeAngles(), &vecDir );

			trace_t tr;

			UTIL_TraceLine( vecSrc, vecSrc + vecDir * 1024, MASK_SOLID_BRUSHONLY, pOwner, COLLISION_GROUP_NONE, &tr );

			CSoundEnt::InsertSound( SOUND_DANGER_SNIPERONLY, tr.endpos, 384, 0.2, pOwner );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponHandgren::Reload( void )
{
	if ( !HasPrimaryAmmo() )
		return false;

	if ( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		//Redraw the weapon
		SendWeaponAnim( ACT_VM_DRAW );

		//Update our times
		m_flNextPrimaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flNextSecondaryAttack	= gpGlobals->curtime + SequenceDuration();
		m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration();

		//Mark this as done
		m_bRedraw = false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHandgren::SecondaryAttack( void )
{
	if ( m_bRedraw )
		return;

	if ( !HasPrimaryAmmo() )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();

	if ( pOwner == NULL )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOwner );
	
	if ( pPlayer == NULL )
		return;

	// Note that this is a secondary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_SECONDARY;
	SendWeaponAnim( ACT_VM_PULLBACK_LOW );

	// Don't let weapon idle interfere in the middle of a throw!
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextSecondaryAttack	= FLT_MAX;

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHandgren::PrimaryAttack( void )
{
	//228
	if ( m_bRedraw )
		return;

	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
	{ 
		return;
	}
	
	//new
	if (m_bIsIronsighted)
	{
		SecondaryAttack();
	}

	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );;

	if ( !pPlayer )
		return;

	// Note that this is a primary attack and prepare the grenade attack to pause.
	m_AttackPaused = GRENADE_PAUSED_PRIMARY;
	SendWeaponAnim( ACT_VM_PULLBACK_HIGH );
	
	// Put both of these off indefinitely. We do not know how long
	// the player will hold the grenade.
	m_flTimeWeaponIdle = FLT_MAX;
	m_flNextPrimaryAttack = FLT_MAX;

	// If I'm now out of ammo, switch away
	if ( !HasPrimaryAmmo() )
	{
		pPlayer->SwitchToNextBestWeapon( this );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOwner - 
//-----------------------------------------------------------------------------
void CWeaponHandgren::DecrementAmmo( CBaseCombatCharacter *pOwner )
{
	pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponHandgren::ItemPostFrame( void )
{

	//Зачем по айронсайту кидать грену?

	/*if (m_bIsIronsighted)
	{
		SecondaryAttack();
	}
	if (m_bIsIronsighted)
	{
		DisableIronsights();
	}
	*/
	if( m_fDrawbackFinished )
	{
		CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

		if (pOwner)
		{
			switch( m_AttackPaused )
			{
			case GRENADE_PAUSED_PRIMARY:
				if( !(pOwner->m_nButtons & IN_ATTACK) )
				{
					SendWeaponAnim( ACT_VM_THROW );
					m_fDrawbackFinished = false;
				}
				break;

			case GRENADE_PAUSED_SECONDARY:
				if( !(pOwner->m_nButtons & IN_ATTACK2) )
				{
					//See if we're ducking
					if ( pOwner->m_nButtons & IN_DUCK )
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_SECONDARYATTACK );
					}
					else
					{
						//Send the weapon animation
						SendWeaponAnim( ACT_VM_HAULBACK );
					}

					m_fDrawbackFinished = false;
				}
				break;

			default:
				break;
			}
		}
	}

	BaseClass::ItemPostFrame();

	if ( m_bRedraw )
	{
		if ( IsViewModelSequenceFinished() )
		{
			Reload();
		}
	}
}

	// check a throw from vecSrc.  If not valid, move the position back along the line to vecEye
void CWeaponHandgren::CheckThrowPosition( CBasePlayer *pPlayer, const Vector &vecEye, Vector &vecSrc )
{
	trace_t tr;

	UTIL_TraceHull( vecEye, vecSrc, -Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), Vector(GRENADE_RADIUS+2,GRENADE_RADIUS+2,GRENADE_RADIUS+2), 
		pPlayer->PhysicsSolidMaskForEntity(), pPlayer, pPlayer->GetCollisionGroup(), &tr );
	
	if ( tr.DidHit() )
	{
		vecSrc = tr.endpos;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHandgren::ThrowGrenade( CBasePlayer *pPlayer )
{
	Vector	vecEye = pPlayer->EyePosition();
	Vector	vForward, vRight;

	pPlayer->EyeVectors( &vForward, &vRight, NULL );
	Vector vecSrc = vecEye + vForward * 18.0f + vRight * 8.0f;
	CheckThrowPosition( pPlayer, vecEye, vecSrc );
//	vForward[0] += 0.1f;
	vForward[2] += 0.1f;

	Vector vecThrow;
	pPlayer->GetVelocity( &vecThrow, NULL );
	vecThrow += vForward * 500; //1200

	//Create the grenade
	QAngle angles;
	VectorAngles(vecThrow, angles);
	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
	pGrenade->SetModel("models/items/ar3_grenade_noshell.mdl");
	pGrenade->SetAbsVelocity(vecThrow);

	pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
	pGrenade->SetLocalAngularVelocity(RandomAngle(-15, 15));
	pGrenade->SetLocalAngularVelocity(QAngle(0, 400, 0));
	pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	pGrenade->SetThrower(GetOwner());
	pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());
	m_bRedraw = true;

	WeaponSound( SINGLE );

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHandgren::LobGrenade( CBasePlayer *pPlayer )
{
	ThrowGrenade(pPlayer);
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponHandgren::RollGrenade( CBasePlayer *pPlayer )
{
	ThrowGrenade(pPlayer);
	return;
}

