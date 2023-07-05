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

class CWeaponMapcase : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeaponMapcase, CBaseHLCombatWeapon);
public:
	int		FrameNumb;

	DECLARE_SERVERCLASS();

	CWeaponMapcase(void);

	void	Spawn(void);
	void	FallInit(void);

	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	void	Drop(const Vector &vecVelocity);
	void	MapcaseStickyTouch(CBaseEntity *pOther);
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

IMPLEMENT_SERVERCLASS_ST(CWeaponMapcase, DT_WeaponMapcase)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_mapcase, CWeaponMapcase);
#ifndef HL2MP
PRECACHE_WEAPON_REGISTER(weapon_mapcase);
#endif

BEGIN_DATADESC(CWeaponMapcase)

DEFINE_FIELD(m_hSporeTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_bRedraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bEmitSpores, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDrawBackFinished, FIELD_BOOLEAN),

DEFINE_FUNCTION(MapcaseStickyTouch),

END_DATADESC()

// Bodygroups
#define BODYGROUP_BADGE 1
#define BODYGROUP_PAPER 3
#define BODYGROUP_INVITE 2
#define BODYGROUP_PHOTO 4
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CWeaponMapcase::CWeaponMapcase(void)
{
	m_bDrawBackFinished = false;
	m_bRedraw = false;
	m_hSporeTrail = NULL;
}
ConVar sde_mission_note_status_skin("sde_mission_note_status_skin", "0");
extern ConVar sde_holster_fixer;
//ConVar sde_mission_note_bodygroup("sde_mission_note_bodygroup", "0");
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::Spawn(void)
{
	BaseClass::Spawn();

	SetSize(Vector(-4, -4, -4), Vector(4, 4, 4));
	CollisionProp()->UseTriggerBounds(true, 100);
}
void CWeaponMapcase::SetSkin(void)
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

	if (sde_mission_note_status_skin.GetInt() >= 32 && sde_mission_note_status_skin.GetInt() <= 56)
		pViewModel->SetBodygroup(BODYGROUP_PHOTO, 1);
	else
		pViewModel->SetBodygroup(BODYGROUP_PHOTO, 0);


	pViewModel->m_nSkin = input / 6;
	pViewModel->SetBodygroup(BODYGROUP_PAPER, input % 6);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::FallInit(void)
{
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
void CWeaponMapcase::Precache(void)
{
	BaseClass::Precache();

	PrecacheScriptSound("NPC_Alyx.PushButton2");

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::Drop(const Vector &vecVelocity)
{
	BaseClass::Drop(vecVelocity);

	// On touch, stick & stop moving. Increase our thinktime a bit so we don't stomp the touch for a bit
	SetNextThink(gpGlobals->curtime + 3.0);
	SetTouch(&CWeaponMapcase::MapcaseStickyTouch);
}

//-----------------------------------------------------------------------------
// Purpose: Stick to the world when we touch it
//-----------------------------------------------------------------------------
void CWeaponMapcase::MapcaseStickyTouch(CBaseEntity *pOther)
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
void CWeaponMapcase::OnPickedUp(CBaseCombatCharacter *pNewOwner)
{
	BaseClass::OnPickedUp(pNewOwner);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::PrimaryAttack(void)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::SecondaryAttack(void)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//-----------------------------------------------------------------------------
void CWeaponMapcase::ThrowGrenade(CBasePlayer *pPlayer)
{
	m_bRedraw = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponMapcase::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
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
bool CWeaponMapcase::Reload(void)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponMapcase::ItemPostFrame(void)
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
	
	/*if (!pOwner->m_bIsCrosshaired)
		pOwner->ShowCrosshair(false);
	*/

	if (sde_holster_fixer.GetInt() == 1) //holster fixer
	{
		if (GetActivity() == ACT_VM_IDLE && HolsterFix && (gpGlobals->curtime > HolsterFixTime))
		{
			SetWeaponVisible(true);
			DevMsg("SDE: holster fixer enabled\n");
			HolsterFix = false;
		}
	}


	DevMsg("PDA:	paper group id	%d \n", FindBodygroupByName("paper"));
	DevMsg("PDA:	badge group id	%d \n", FindBodygroupByName("badge"));
	DevMsg("PDA:	invite group id	%d \n", FindBodygroupByName("invite"));
	DevMsg("PDA:	photo group id	%d \n", FindBodygroupByName("photo"));

	SetSkin();
	WeaponIdle();
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMapcase::Deploy(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer == NULL)
		return false;
	pPlayer->ShowCrosshair(false);
	DisplaySDEHudHint(); //added
	SetSkin();

	m_bRedraw = false;
	m_bDrawBackFinished = false;
	SetSkin();
	HolsterFix = true;
	HolsterFixTime = (gpGlobals->curtime + 1.5f); //holster fixer
	//CBaseViewModel *pViewModel = pOwner->GetViewModel();
	//pViewModel->set
	//
	//
	return BaseClass::Deploy();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CWeaponMapcase::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	m_bRedraw = false;
	m_bDrawBackFinished = false;
	SetSkin();

	return BaseClass::Holster(pSwitchingTo);
}