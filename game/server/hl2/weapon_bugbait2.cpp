//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "engine/IEngineSound.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "antlion_maker.h"
#include "grenade_bugbait.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
// Bug Bait Weapon
//

class CWeaponBugBait2 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponBugBait2, CBaseHLCombatWeapon);
public:

	DECLARE_SERVERCLASS();

	CWeaponBugBait2(void);

	void	Spawn(void);
	void	FallInit(void);

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	void	Drop(const Vector &vecVelocity);
	void	BugbaitStickyTouch(CBaseEntity *pOther);
	void	OnPickedUp(CBaseCombatCharacter *pNewOwner);
	bool	Deploy(void);
	bool	Holster(CBaseCombatWeapon *pSwitchingTo);

	void	ItemPostFrame(void);
	void	Precache(void);
	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	ThrowGrenade(CBasePlayer *pPlayer);

	bool	HasAnyAmmo(void) { return true; }

	bool	Reload(void);

	void	SetSporeEmitterState(bool state = true);

	bool	ShouldDisplayHUDHint() { return true; }

	DECLARE_DATADESC();

protected:

	bool		m_bDrawBackFinished;
	bool		m_bRedraw;
	bool		m_bEmitSpores;
	EHANDLE		m_hSporeTrail;
private:
	void	SetSkin(int skinNum);
	void	SetSkin2(void);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBugBait2, DT_WeaponBugBait2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_bugbait2, CWeaponBugBait2);
#ifndef HL2MP
PRECACHE_WEAPON_REGISTER(weapon_bugbait2);
#endif

BEGIN_DATADESC(CWeaponBugBait2)

DEFINE_FIELD(m_hSporeTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bEmitSpores, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDrawBackFinished, FIELD_BOOLEAN),

DEFINE_FUNCTION(BugbaitStickyTouch),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponBugBait2::CWeaponBugBait2(void)
{
	m_bDrawBackFinished = false;
	m_bRedraw = false;
	m_hSporeTrail = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//ConVar sde_mission_note_status_skin("sde_mission_note_status_skin", "0");
void CWeaponBugBait2::Spawn(void)
{
	BaseClass::Spawn();

	// Increase the bugbait's pickup volume. It spawns inside the antlion guard's body,
	// and playtesters seem to be wary about moving into the body.
	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));
	CollisionProp()->UseTriggerBounds(true, 100);
}
void CWeaponBugBait2::SetSkin(int skinNum)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	pViewModel->m_nSkin = skinNum;
}
void CWeaponBugBait2::SetSkin2(void)
{
	//CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	//
	//if (pOwner == NULL)
	//	return;
	//
	//CBaseViewModel *pViewModel = pOwner->GetViewModel();
	//
	//if (pViewModel == NULL)
	//	return;
	//
	//pViewModel->m_nSkin = sde_mission_note_status_skin.GetFloat();
}
//void SetSkin3(CBasePlayer *owner, Vector &pos, QAngle &ang)
//{
//	if (sde_mission_note_status_skin.GetFloat())
//	{
//		CBasePlayer *pOwner = ToBasePlayer(GetOwner());
//		if (pOwner == NULL)
//		return;
//
//		CBaseViewModel *pViewModel = pOwner->GetViewModel();
//		if (pViewModel == NULL)
//		return;
//		pViewModel->m_nSkin = sde_mission_note_status_skin.GetFloat();
//	}
//}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::FallInit(void)
{
	// Bugbait shouldn't be physics, because it musn't roll/move away from it's spawnpoint.
	// The game will break if the player can't pick it up, so it must stay still.
	SetModel(GetWorldModel());

	VPhysicsDestroyObject();
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER);

	SetPickupTouch();

	SetThink(&CBaseCombatWeapon::FallThink);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("npc_grenade_bugbait");

	PrecacheScriptSound("Weapon_Bugbait.Splat");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::Drop(const Vector &vecVelocity)
{
	BaseClass::Drop(vecVelocity);

	// On touch, stick & stop moving. Increase our thinktime a bit so we don't stomp the touch for a bit
	SetNextThink(gpGlobals->curtime + 3.0);
	SetTouch(&CWeaponBugBait2::BugbaitStickyTouch);

	//m_hSporeTrail = SporeExplosion::CreateSporeExplosion();
	//if ( m_hSporeTrail )
	//{
	//	SporeExplosion *pSporeExplosion = (SporeExplosion *)m_hSporeTrail.Get();
	//
	//	QAngle	angles;
	//	VectorAngles( Vector(0,0,1), angles );
	//
	//	pSporeExplosion->SetAbsAngles( angles );
	//	pSporeExplosion->SetAbsOrigin( GetAbsOrigin() );
	//	pSporeExplosion->SetParent( this );
	//
	//	pSporeExplosion->m_flSpawnRate			= 16.0f;
	//	pSporeExplosion->m_flParticleLifetime	= 0.5f;
	//	pSporeExplosion->SetRenderColor( 0.0f, 0.5f, 0.25f, 0.15f );
	//
	//	pSporeExplosion->m_flStartSize			= 32;
	//	pSporeExplosion->m_flEndSize			= 48;
	//	pSporeExplosion->m_flSpawnRadius		= 4;
	//
	//	pSporeExplosion->SetLifetime( 9999 );
	//}
}

//-----------------------------------------------------------------------------
// Purpose: Stick to the world when we touch it
//-----------------------------------------------------------------------------
void CWeaponBugBait2::BugbaitStickyTouch(CBaseEntity *pOther)
{
	if (!pOther->IsWorld())
		return;

	// Stop moving, wait for pickup
	SetMoveType(MOVETYPE_NONE);
	SetThink(NULL);
	SetPickupTouch();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
	BaseClass::OnPickedUp(pNewOwner);

	if (m_hSporeTrail)
	{
		UTIL_Remove(m_hSporeTrail);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::PrimaryAttack(void)
{
	//SetSkin( 1 );
	//if ( m_bRedraw )
	return;

	//CBaseCombatCharacter *pOwner  = GetOwner();
	//
	//if ( pOwner == NULL )
	//	return;
	//
	//CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );
	//
	//if ( pPlayer == NULL )
	//	return;
	//
	//SendWeaponAnim( ACT_VM_HAULBACK );
	//
	//m_flTimeWeaponIdle		= FLT_MAX;
	//m_flNextPrimaryAttack	= FLT_MAX;
	//
	//m_iPrimaryAttacks++;
	//gamestats->Event_WeaponFired( pPlayer, true, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::SecondaryAttack(void)
{
	// Squeeze!
	//SetSkin(0);
	//CPASAttenuationFilter filter( this );
	//
	//EmitSound( filter, entindex(), "Weapon_Bugbait.Splat" );
	//
	//if ( CGrenadeBugBait::ActivateBugbaitTargets( GetOwner(), GetAbsOrigin(), true ) == false )
	//{
	//	g_AntlionMakerManager.BroadcastFollowGoal( GetOwner() );
	//}
	//
	//SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	//m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	//
	//CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	//if ( pOwner )
	//{
	//	m_iSecondaryAttacks++;
	//	gamestats->Event_WeaponFired( pOwner, false, GetClassname() );
	//}
	return;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::ThrowGrenade(CBasePlayer *pPlayer)
{
	Vector	vForward, vRight, vUp, vThrowPos, vThrowVel;

	pPlayer->EyeVectors(&vForward, &vRight, &vUp);

	vThrowPos = pPlayer->EyePosition();

	vThrowPos += vForward * 18.0f;
	vThrowPos += vRight * 12.0f;

	pPlayer->GetVelocity(&vThrowVel, NULL);
	vThrowVel += vForward * 1000;

	CGrenadeBugBait *pGrenade = BugBaitGrenade_Create(vThrowPos, vec3_angle, vThrowVel, QAngle(600, random->RandomInt(-1200, 1200), 0), pPlayer);

	if (pGrenade != NULL)
	{
		// If the shot is clear to the player, give the missile a grace period
		trace_t	tr;
		UTIL_TraceLine(pPlayer->EyePosition(), pPlayer->EyePosition() + (vForward * 128), MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

		if (tr.fraction == 1.0)
		{
			pGrenade->SetGracePeriod(0.1f);
		}
	}

	m_bRedraw = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SEQUENCE_FINISHED:
		m_bDrawBackFinished = true;
		break;

	case EVENT_WEAPON_THROW:
		ThrowGrenade(pOwner);
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
	SetSkin2();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponBugBait2::Reload(void)
{
	if ((m_bRedraw) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		//Redraw the weapon
		SendWeaponAnim(ACT_VM_DRAW);

		//Update our times
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		 
		//Mark this as done
		m_bRedraw = false;
	}
	SetSkin2();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// See if we're cocked and ready to throw
	if (m_bDrawBackFinished)
	{
		if ((pOwner->m_nButtons & IN_ATTACK) == false)
		{
			SendWeaponAnim(ACT_VM_THROW);
			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
			m_bDrawBackFinished = false;
		}
	}
	else
	{
		//See if we're attacking
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack < gpGlobals->curtime))
		{
			PrimaryAttack();
		}
		else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack < gpGlobals->curtime))
		{
			SecondaryAttack();
		}
	}

	if (m_bRedraw)
	{
		if (IsViewModelSequenceFinished())
		{
			Reload();
		}
	}

	WeaponIdle();
	SetSkin2();
	if (m_bIsIronsighted)
	{
		DisableIronsights();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponBugBait2::Deploy(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return false;
	SetSkin2();
	/*
	if ( m_hSporeTrail == NULL )
	{
	m_hSporeTrail = SporeTrail::CreateSporeTrail();

	m_hSporeTrail->m_bEmit				= true;
	m_hSporeTrail->m_flSpawnRate		= 100.0f;
	m_hSporeTrail->m_flParticleLifetime	= 2.0f;
	m_hSporeTrail->m_flStartSize		= 1.0f;
	m_hSporeTrail->m_flEndSize			= 4.0f;
	m_hSporeTrail->m_flSpawnRadius		= 8.0f;

	m_hSporeTrail->m_vecEndColor		= Vector( 0, 0, 0 );

	CBaseViewModel *vm = pOwner->GetViewModel();

	if ( vm != NULL )
	{
	m_hSporeTrail->FollowEntity( vm );
	}
	}
	*/

	m_bRedraw = false;
	m_bDrawBackFinished = false;

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponBugBait2::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bRedraw = false;
	m_bDrawBackFinished = false;
	SetSkin2();
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : true - 
//-----------------------------------------------------------------------------
void CWeaponBugBait2::SetSporeEmitterState(bool state)
{
	m_bEmitSpores = state;
}
