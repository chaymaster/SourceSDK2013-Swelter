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
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//
// it was Bug Bait Weapon but now it mapcase
//

class CWeaponBugBait : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponBugBait, CBaseHLCombatWeapon);
public:
	int		FrameNumb;

	DECLARE_SERVERCLASS();

	CWeaponBugBait(void);

	void	Spawn(void);
	void	FallInit(void);

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
	void	SetSkin( void );
};

IMPLEMENT_SERVERCLASS_ST(CWeaponBugBait, DT_WeaponBugBait)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_bugbait, CWeaponBugBait);
#ifndef HL2MP
PRECACHE_WEAPON_REGISTER(weapon_bugbait);
#endif

BEGIN_DATADESC(CWeaponBugBait)

DEFINE_FIELD(m_hSporeTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bEmitSpores, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDrawBackFinished, FIELD_BOOLEAN),

DEFINE_FUNCTION(BugbaitStickyTouch),

END_DATADESC()

// Bodygroups
#define BODYGROUP_BADGE 1
#define BODYGROUP_PAPER 3
#define BODYGROUP_INVITE 2
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponBugBait::CWeaponBugBait(void)
{
	m_bDrawBackFinished = false;
	m_bRedraw = false;
	m_hSporeTrail = NULL;
}
ConVar sde_mission_note_status_skin("sde_mission_note_status_skin", "0");
//ConVar sde_mission_note_bodygroup("sde_mission_note_bodygroup", "0");
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::Spawn(void)
{
	BaseClass::Spawn();

	// Increase the bugbait's pickup volume. It spawns inside the antlion guard's body,
	// and playtesters seem to be wary about moving into the body.
	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));
	CollisionProp()->UseTriggerBounds(true, 100);
}
void CWeaponBugBait::SetSkin(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	int input = sde_mission_note_status_skin.GetInt();


	if (sde_mission_note_status_skin.GetInt() <= 38)
		pViewModel->SetBodygroup(BODYGROUP_BADGE, 1);
	else
		pViewModel->SetBodygroup(BODYGROUP_BADGE, 0);

	if (sde_mission_note_status_skin.GetInt() >= 32)
		pViewModel->SetBodygroup(BODYGROUP_INVITE, 1);
	else
		pViewModel->SetBodygroup(BODYGROUP_INVITE, 0);


	pViewModel->m_nSkin = input / 6;
	pViewModel->SetBodygroup(BODYGROUP_PAPER, input % 6);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::FallInit(void)
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
void CWeaponBugBait::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("Weapon_Bugbait.Splat");
	PrecacheScriptSound("NPC_Alyx.PushButton2");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::Drop(const Vector &vecVelocity)
{
	BaseClass::Drop(vecVelocity);

	// On touch, stick & stop moving. Increase our thinktime a bit so we don't stomp the touch for a bit
	SetNextThink(gpGlobals->curtime + 3.0);
	SetTouch(&CWeaponBugBait::BugbaitStickyTouch);
}

//-----------------------------------------------------------------------------
// Purpose: Stick to the world when we touch it
//-----------------------------------------------------------------------------
void CWeaponBugBait::BugbaitStickyTouch(CBaseEntity *pOther)
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
void CWeaponBugBait::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
	BaseClass::OnPickedUp(pNewOwner);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::PrimaryAttack(void)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::SecondaryAttack(void)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponBugBait::ThrowGrenade(CBasePlayer *pPlayer)
{
	m_bRedraw = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponBugBait::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
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
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponBugBait::Reload(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponBugBait::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	if (m_bIsIronsighted)
	{
		DisableIronsights();
		SecondaryAttack();
	}

	DevMsg("PDA:	paper group id	%d \n", FindBodygroupByName("paper"));
	DevMsg("PDA:	badge group id	%d \n", FindBodygroupByName("badge"));
	DevMsg("PDA:	invite group id	%d \n", FindBodygroupByName("invite"));

	SetSkin();
	WeaponIdle();
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponBugBait::Deploy(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return false;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(false);
	DisplaySDEHudHint(); //added
	SetSkin();

	m_bRedraw = false;
	m_bDrawBackFinished = false;
	SetSkin();

	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponBugBait::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bRedraw = false;
	m_bDrawBackFinished = false;
	SetSkin();

	return BaseClass::Holster(pSwitchingTo);
}
