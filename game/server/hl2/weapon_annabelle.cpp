//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A shotgun.
//
//			Primary attack: single barrel shot.
//			Secondary attack: double barrel shot.
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "basehlcombatweapon_shared.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include <hl2_player.h>
#include "gamerules.h"		// For g_pGameRules
#include "in_buttons.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar sk_auto_reload_time;
extern ConVar sk_plr_num_shotgun_pellets;
extern ConVar sde_simple_rifle_bolt;
extern ConVar sde_holster_fixer;

class CWeaponAnnabelle : public CBaseHLCombatWeapon
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponAnnabelle, CBaseHLCombatWeapon);

	DECLARE_SERVERCLASS();

private:
	bool	m_bNeedPump;		// After shot
	bool	m_bDelayedFire1;	// Fire primary when finished reloading
	bool	m_bDelayedFire2;	// Fire secondary when finished reloading
	bool	m_bEjectChamberedRound;
	bool	m_bNeedToCloseChamber;
	bool	m_bCompensateEjectedRoundForFullAmmoSupply;
	float	m_flTimeToSubtractEjectedChamberedRound;
	float	m_flReloadEnd;	// when all animations on ending the reload actually end

public:
	void	Precache(void);

	int CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector vitalAllyCone = VECTOR_CONE_5DEGREES;
		static Vector cone = VECTOR_CONE_5DEGREES;

		if (GetOwner() && (GetOwner()->Classify() == CLASS_PLAYER_ALLY_VITAL))
		{
			// Give Alyx's shotgun blasts more a more directed punch. She needs
			// to be at least as deadly as she would be with her pistol to stay interesting (sjb)
			return vitalAllyCone;
		}

		return cone;
	}

	virtual int				GetMinBurst() { return 1; }
	virtual int				GetMaxBurst() { return 3; }

	virtual float			GetMinRestTime();
	virtual float			GetMaxRestTime();

	virtual float			GetFireRate(void);

	bool StartReload(void);
	bool Reload(void);
	void FillClip(void);
	void FinishReload(void);
	void CheckHolsterReload(void);
	void Pump(void);
	//	void WeaponIdle( void );
	void ItemHolsterFrame(void);
	void ItemPostFrame(void);
	void PrimaryAttack(void);
	void HoldIronsight(void);
	void SecondaryAttack(void);
	void DryFire(void);
	bool Deploy(void);

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

	CWeaponAnnabelle(void);
};

IMPLEMENT_SERVERCLASS_ST(CWeaponAnnabelle, DT_WeaponAnnabelle)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_annabelle, CWeaponAnnabelle);
PRECACHE_WEAPON_REGISTER(weapon_annabelle);

BEGIN_DATADESC(CWeaponAnnabelle)
DEFINE_FIELD(m_bNeedPump, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFire1, FIELD_BOOLEAN),
DEFINE_FIELD(m_bDelayedFire2, FIELD_BOOLEAN),
DEFINE_FIELD(m_bEjectChamberedRound, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedToCloseChamber, FIELD_BOOLEAN),
DEFINE_FIELD(m_bCompensateEjectedRoundForFullAmmoSupply, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTimeToSubtractEjectedChamberedRound, FIELD_TIME),
DEFINE_FIELD(m_flReloadEnd, FIELD_TIME),
END_DATADESC()

acttable_t	CWeaponAnnabelle::m_acttable[] =
{
	{ ACT_IDLE, ACT_IDLE_SMG1, true },	// FIXME: hook to shotgun unique

	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RELOAD, ACT_RELOAD_SHOTGUN, false },
	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SHOTGUN, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SHOTGUN_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SHOTGUN_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_SHOTGUN_AGITATED, false },//always aims

	{ ACT_WALK_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_STIMULATED, ACT_WALK_RIFLE_STIMULATED, false },
	{ ACT_WALK_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_STIMULATED, ACT_RUN_RIFLE_STIMULATED, false },
	{ ACT_RUN_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims

	// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED, ACT_IDLE_AIM_RIFLE_STIMULATED, false },
	{ ACT_IDLE_AIM_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

	{ ACT_WALK_AIM_RELAXED, ACT_WALK_RIFLE_RELAXED, false },//never aims
	{ ACT_WALK_AIM_STIMULATED, ACT_WALK_AIM_RIFLE_STIMULATED, false },
	{ ACT_WALK_AIM_AGITATED, ACT_WALK_AIM_RIFLE, false },//always aims

	{ ACT_RUN_AIM_RELAXED, ACT_RUN_RIFLE_RELAXED, false },//never aims
	{ ACT_RUN_AIM_STIMULATED, ACT_RUN_AIM_RIFLE_STIMULATED, false },
	{ ACT_RUN_AIM_AGITATED, ACT_RUN_AIM_RIFLE, false },//always aims
	//End readiness activities

	{ ACT_WALK_AIM, ACT_WALK_AIM_SHOTGUN, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_SHOTGUN, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SHOTGUN, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SHOTGUN_LOW, true },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SHOTGUN_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SHOTGUN, false },
};

IMPLEMENT_ACTTABLE(CWeaponAnnabelle);

void CWeaponAnnabelle::Precache(void)
{
	CBaseCombatWeapon::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);
	WeaponSound(SINGLE_NPC);
	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
		AngleVectors(angShootDir, &vecShootDir);
	}
	else
	{
		vecShootOrigin = pOperator->Weapon_ShootPosition();
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);
	}

	pOperator->FireBullets(8, vecShootOrigin, vecShootDir, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	FireNPCPrimaryAttack(pOperator, true);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SHOTGUN_FIRE:
	{
									  FireNPCPrimaryAttack(pOperator, false);
	}
		break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose:	When we shipped HL2, the shotgun weapon did not override the
//			BaseCombatWeapon default rest time of 0.3 to 0.6 seconds. When
//			NPC's fight from a stationary position, their animation events
//			govern when they fire so the rate of fire is specified by the
//			animation. When NPC's move-and-shoot, the rate of fire is 
//			specifically controlled by the shot regulator, so it's imporant
//			that GetMinRestTime and GetMaxRestTime are implemented and provide
//			reasonable defaults for the weapon. To address difficulty concerns,
//			we are going to fix the combine's rate of shotgun fire in episodic.
//			This change will not affect Alyx using a shotgun in EP1. (sjb)
//-----------------------------------------------------------------------------
float CWeaponAnnabelle::GetMinRestTime()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
	{
		return 1.2f;
	}

	return BaseClass::GetMinRestTime();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CWeaponAnnabelle::GetMaxRestTime()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
	{
		return 1.5f;
	}

	return BaseClass::GetMaxRestTime();
}

//-----------------------------------------------------------------------------
// Purpose: Time between successive shots in a burst. Also returned for EP2
//			with an eye to not messing up Alyx in EP1.
//-----------------------------------------------------------------------------
float CWeaponAnnabelle::GetFireRate()
{
	if (hl2_episodic.GetBool() && GetOwner() && GetOwner()->Classify() == CLASS_COMBINE)
	{
		return 0.8f;
	}

	return 0.7;
}
bool CWeaponAnnabelle::Deploy(void)
{
	DevMsg("SDE_SMG!_deploy\n");
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	DisplaySDEHudHint();
	HolsterFix = true;
	HolsterFixTime = (gpGlobals->curtime + 1.5f); //holster fixer

	bool return_value = BaseClass::Deploy();

	if ((!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType)) || m_bNeedToCloseChamber)
	{
		m_flReloadEnd = gpGlobals->curtime + SequenceDuration() + 0.1f; // a little past deploy animation as reloading will follow
		m_bForbidIronsight = true; // to suppress toggle ironsight if reload or chamber closing follows
	}

	return return_value;
}
//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponAnnabelle::StartReload(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (!pPlayer)
	{
		return false;
	}

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	DisableIronsights();

	// If shotgun totally emptied then a pump animation is needed

	//NOTENOTE: This is kinda lame because the player doesn't get strong feedback on when the reload has finished,
	//			without the pump.  Technically, it's incorrect, but it's good for feedback...

	//if (m_iClip1 <= 0)
	//{
	//	m_bNeedPump = true;
	//}

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	SendWeaponAnim(ACT_SHOTGUN_RELOAD_START);

	// Make shotgun shell visible
	SetBodygroup(1, 0);

	m_bEjectChamberedRound = m_bCompensateEjectedRoundForFullAmmoSupply = false; // initialize trigger chain each reload

	if (m_iClip1 >= 1 && (sde_simple_rifle_bolt.GetInt() || (!sde_simple_rifle_bolt.GetInt() && pHL2Player->Get_Annabelle_Chamber())))
	{ //don't split this trick into eject-catch separated in time if player has maximum spare ammo, not to lose a round 
		m_flTimeToSubtractEjectedChamberedRound = gpGlobals->curtime + 0.5f;
		// the ejected round will be caught to the inventory in case of simple bolting or discarded in case of manual bolting
		m_bEjectChamberedRound = true;
		if (!sde_simple_rifle_bolt.GetInt())
			pHL2Player->Annabelle_Round_Unchamber();
	}

	if (m_iClip1 < 1)
	{
		m_bBoltRequired = true;
	}

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flReloadEnd = m_flNextPrimaryAttack + 0.1f; // moved a little past primary attack

	m_bNeedToCloseChamber = true;
	m_bInReload = true;
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponAnnabelle::Reload(void)
{

	// Check that StartReload was called first
	if (!m_bInReload)
	{
		Warning("ERROR: Shotgun Reload called incorrectly!\n");
	}

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return false;

	if (pOwner->IsPlayer())
		((CBasePlayer*)pOwner)->ShowCrosshair(true); // show crosshair to fix crosshair for reloading weapons in toggle ironsight

	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		return false;

	if (m_iClip1 >= GetMaxClip1())
		return false;

	DisableIronsights();

	int j = MIN(1, pOwner->GetAmmoCount(m_iPrimaryAmmoType));

	if (j <= 0)
		return false;

	FillClip();
	// Play reload on different channel as otherwise steals channel away from fire sound
	WeaponSound(RELOAD);
	SendWeaponAnim(ACT_VM_RELOAD);

	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flReloadEnd = m_flNextPrimaryAttack + 0.1f; // moved every loaded round a little past primary attack

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::FinishReload(void)
{
	// Make shotgun shell invisible
	SetBodygroup(1, 1);

	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (!pPlayer)
	{
		return;
	}

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	m_bNeedPump = false;
	m_bNeedToCloseChamber = false;

	//if (!sde_simple_rifle_bolt.GetInt())
	pHL2Player->Annabelle_Round_Chamber(); // always chamber the round on finishing reload, to prevent glitch on switching between auto/manual bolt

	DisableIronsights();

	// Finish reload animation
	SendWeaponAnim(ACT_SHOTGUN_RELOAD_FINISH);
	pOwner->m_flNextAttack = gpGlobals->curtime;
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flReloadEnd = m_flNextPrimaryAttack; // finishing reload precisely as the primary attack is ready
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::FillClip(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	// Add them to the clip
	if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) > 0)
	{
		if (Clip1() < GetMaxClip1())
		{
			m_iClip1++;
			if (m_bCompensateEjectedRoundForFullAmmoSupply && pOwner->GetAmmoCount(m_iPrimaryAmmoType) >= pOwner->GetMaxCarry(m_iPrimaryAmmoType))
				// bolt action realism in case of full supply of spare ammo, no place for compensation round - just don't subtract it
			{
				m_bCompensateEjectedRoundForFullAmmoSupply = false;
			}
			else
			{
				pOwner->RemoveAmmo(1, m_iPrimaryAmmoType);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play weapon pump anim
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::Pump(void)
{
	//HoldIronsight();
	CBaseCombatCharacter *pOwner = GetOwner();

	if (pOwner == NULL)
		return;

	CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	if (!pPlayer)
	{
		return;
	}

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	m_bNeedPump = false;

	//if (!sde_simple_rifle_bolt.GetInt())
	pHL2Player->Annabelle_Round_Chamber();

	WeaponSound(SPECIAL1);

	// Finish reload animation

	SendWeaponAnim(ACT_SHOTGUN_PUMP);

	pOwner->m_flNextAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::DryFire(void)
{
	WeaponSound(EMPTY);
	SendWeaponAnim(ACT_VM_DRYFIRE);

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (!sde_simple_rifle_bolt.GetInt() && !pHL2Player->Get_Annabelle_Chamber())
	{
		Pump();
	}

	else
	{

		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE);

		pPlayer->DoMuzzleFlash();

		if (m_bIsIronsighted)
		{
			SendWeaponAnim(ACT_VM_IRONSHOOT);
			pPlayer->ViewPunch(QAngle(random->RandomFloat(-8, -4), random->RandomFloat(-6, 6), 0));
			//pPlayer->ViewPunch(QAngle(random->RandomFloat(-4, -2), random->RandomFloat(-4, 4), 0)); punch off
		}
		else
		{
			SendWeaponAnim(ACT_VM_PRIMARYATTACK);
			pPlayer->ViewPunch(QAngle(random->RandomFloat(-12, -8), random->RandomFloat(-10, 10), 0));

		}

		// player "shoot" animation
		pPlayer->SetAnimation(PLAYER_ATTACK1);

		// Don't fire again until fire animation has completed
		m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		m_iClip1 -= 1;

		Vector	vecSrc = pPlayer->Weapon_ShootPosition();
		Vector	vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

		pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 1.0); //suda posmitret pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );

		// Fire the bullets, and force the first shot to be perfectly accuracy
		if (m_bIsIronsighted)
		{
			pPlayer->FireBullets(1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
		}
		else
		{
			pPlayer->FireBullets(1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);
		}


		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2, GetOwner());

		if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
		{
			// HEV suit - indicate out of ammo condition
			pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
		}

		if (m_iClip1 /*&& sde_simple_rifle_bolt.GetInt()*/)
		{
			// pump so long as some rounds are left.
			m_bNeedPump = true;
		}

		if (!sde_simple_rifle_bolt.GetInt())
			pHL2Player->Annabelle_Round_Unchamber(); //even if 0 ammo remains, after shot there is no chambered round

		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, true, GetClassname());
	}
}

void CWeaponAnnabelle::HoldIronsight(void)
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

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::SecondaryAttack(void)
{
	//// Only the player fires this way so we can cast
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}
	
	ToggleIronsights();
	pOwner->ToggleCrosshair();

	//
	//pPlayer->m_nButtons &= ~IN_ATTACK2;
	//// MUST call sound before removing a round from the clip of a CMachineGun
	//WeaponSound(WPN_DOUBLE);
	//
	//pPlayer->DoMuzzleFlash();
	//
	//SendWeaponAnim( ACT_VM_SECONDARYATTACK );
	//
	//// player "shoot" animation
	//pPlayer->SetAnimation( PLAYER_ATTACK1 );
	//
	//// Don't fire again until fire animation has completed
	//m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	//m_iClip1 -= 2;	// Shotgun uses same clip for primary and secondary attacks
	//
	//Vector vecSrc	 = pPlayer->Weapon_ShootPosition();
	//Vector vecAiming = pPlayer->GetAutoaimVector( AUTOAIM_SCALE_DEFAULT );	
	//
	//// Fire the bullets
	//pPlayer->FireBullets( 12, vecSrc, vecAiming, GetBulletSpread(), MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0, -1, -1, 0, NULL, false, false );
	//pPlayer->ViewPunch( QAngle(random->RandomFloat( -5, 5 ),0,0) );
	//
	//pPlayer->SetMuzzleFlashTime( gpGlobals->curtime + 1.0 );
	//
	//CSoundEnt::InsertSound( SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_SHOTGUN, 0.2 );
	//
	//if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	//{
	//	// HEV suit - indicate out of ammo condition
	//	pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0); 
	//}
	//
	//if( m_iClip1 )
	//{
	//	// pump so long as some rounds are left.
	//	m_bNeedPump = true;
	//}
	//
	//m_iSecondaryAttacks++;
	//gamestats->Event_WeaponFired( pPlayer, false, GetClassname() );
}

//-----------------------------------------------------------------------------
// Purpose: Override so shotgun can do multiple reloads in a row
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::ItemPostFrame(void)
{
	// HoldIronsight(); // moved to where the weapon is not in reload
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	// CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pOwner);

	if (sde_holster_fixer.GetInt() == 1) //holster fixer
	{
		if (GetActivity() == ACT_VM_IDLE && HolsterFix && (gpGlobals->curtime > HolsterFixTime))
		{
			SetWeaponVisible(true);
			DevMsg("SDE: holster fixer enabled\n");
			HolsterFix = false;
		}
	}

	if (m_bEjectChamberedRound && gpGlobals->curtime >= m_flTimeToSubtractEjectedChamberedRound)
	{
		m_iClip1--;
		m_bEjectChamberedRound = false;
		// the bolt ejects the chambered round even if it's not an empty casing.
		// In case of simple bolting spare that ejected round to re-insert it later
		if (sde_simple_rifle_bolt.GetInt())
		{
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) < pOwner->GetMaxCarry(m_iPrimaryAmmoType))
			{ // if ammo supply is full, just don't decrease it later when loading the first round
					pOwner->GiveAmmo(1, m_iPrimaryAmmoType, true); // Realism of bolt-action rifle mechanics: when you start reloading
			}
			else
			{
				m_bCompensateEjectedRoundForFullAmmoSupply = true;
			}
		}
	}

	if (GetActivity() == ACT_VM_HOLSTER) //new
	{
		m_flNextPrimaryAttack = gpGlobals->curtime + 1.25f; //new
	}

	if (m_bInReload)
	{
		if (m_flReloadEnd <= gpGlobals->curtime)
		{
			m_bInReload = false; // moved here to not happen before chamber close animation finishes
			return;
		}
		// If I'm primary firing and have one round stop reloading and fire
		if ((pOwner->m_nButtons & IN_ATTACK) && (m_iClip1 >= 1) && m_bNeedToCloseChamber) // to avoid double closing on click during closing
		{
			m_bNeedPump = false;
			m_bDelayedFire1 = true;
		}
		else if (!m_bDelayedFire1 && m_flNextPrimaryAttack <= gpGlobals->curtime)
		{
			// If out of ammo end reload
			if (pOwner->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
			{
				FinishReload();
				return;
			}
			// If clip not full reload again
			if (m_iClip1 < GetMaxClip1())
			{
				Reload();
				return;
			}
			// Clip full, stop reloading
			else
			{
				FinishReload();
				return;
			}
		}
	}
	else
	{
		// Make shotgun shell invisible
		SetBodygroup(1, 1);

		if (m_bForbidIronsight && m_flReloadEnd <= gpGlobals->curtime)
			m_bForbidIronsight = false;

		if (m_iClip1 && m_bNeedToCloseChamber) // for holster in reload + re-equip weapon sequence to handle correctly and load animations to complete
		{
			m_bNeedToCloseChamber = false;
			m_bDelayedFire1 = true; // triggers finish reload after re-equip
			m_bInReload = true; // to prevent ironsight in chamber closing sequence
		}

		if (!(m_bInReload || m_bForbidIronsight || GetActivity() == ACT_VM_HOLSTER))
			HoldIronsight();

		if ((pOwner->m_afButtonPressed & IN_ATTACK2) && (m_flReloadEnd <= gpGlobals->curtime) && (m_flNextPrimaryAttack <= gpGlobals->curtime)) // toggle zoom on mission-critical sniper weapon like vanilla HL2 crossbow
		{
			SecondaryAttack();
		}
	}

	if (m_bNeedPump && m_iClip1 && sde_simple_rifle_bolt.GetInt() && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{ //m_bNeedPump is only true when m_iClip1 >= 1, but let's keep it here for readability
		Pump();
		return;
	}

	if ((m_bDelayedFire1 || pOwner->m_nButtons & IN_ATTACK ) && m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		if ((m_iClip1 <= 0 && UsesClipsForAmmo1()) || (!UsesClipsForAmmo1() && !pOwner->GetAmmoCount(m_iPrimaryAmmoType)))
		{
			if (!pOwner->GetAmmoCount(m_iPrimaryAmmoType))
			{
				DryFire();
			}
			else
			{
				StartReload();
			}
		}
		// Fire underwater?
		else if (pOwner->GetWaterLevel() == 3 && m_bFiresUnderwater == false)
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.2;
			return;
		}
		else
		{
			if (m_bDelayedFire1)
			{
				m_bDelayedFire1 = false;
				FinishReload();
				return;
			}
			else
			{
				// If the firing button was just pressed, reset the firing time
				if (pOwner->m_afButtonPressed & IN_ATTACK)
				{
					m_flNextPrimaryAttack = gpGlobals->curtime;
				}
				PrimaryAttack();
			}
		}
	}

	if (pOwner->m_nButtons & IN_RELOAD && UsesClipsForAmmo1() && !m_bInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		StartReload();
	}
	else
	{
		// no fire buttons down
		m_bFireOnEmpty = false;

		if (!HasAnyAmmo() && m_flNextPrimaryAttack < gpGlobals->curtime)
		{
			// weapon isn't useable, switch.
			if (!(GetWeaponFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && pOwner->SwitchToNextBestWeapon(this))
			{
				m_flNextPrimaryAttack = gpGlobals->curtime + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip1 <= 0 && !(GetWeaponFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < gpGlobals->curtime)
			{
				if (StartReload())
				{
					// if we've successfully started to reload, we're done
					return;
				}
			}
		}

		WeaponIdle();
		return;
	}

}



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponAnnabelle::CWeaponAnnabelle(void)
{
	m_bReloadsSingly = true;

	m_bNeedPump = false;
	m_bDelayedFire1 = false;
	m_bDelayedFire2 = false;

	m_fMinRange1 = 0.0;
	m_fMaxRange1 = 500;
	m_fMinRange2 = 0.0;
	m_fMaxRange2 = 200;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAnnabelle::ItemHolsterFrame(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();

	// Must be player held
	if (pOwner && pOwner->IsPlayer() == false)
		return;

	// We can't be active
	if (pOwner->GetActiveWeapon() == this)
		return;

	/*CBasePlayer *pPlayer = ToBasePlayer(pOwner);

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (m_bInReload && m_iClip1)
		pHL2Player->Annabelle_Round_Chamber(); // for holster in reload + re-equip weapon sequence to handle correctly
	*/

	// If it's been longer than three seconds, reload
	if ((gpGlobals->curtime - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
	{
		// Reset the timer
		m_flHolsterTime = gpGlobals->curtime;

		if (GetOwner() == NULL)
			return;

		if (m_iClip1 == GetMaxClip1())
			return;

		// Just load the clip with no animations
		int ammoFill = MIN((GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount(GetPrimaryAmmoType()));

		GetOwner()->RemoveAmmo(ammoFill, GetPrimaryAmmoType());
		m_iClip1 += ammoFill;
	}
}

//==================================================
// Purpose: 
//==================================================
/*
void CWeaponAnnabelle::WeaponIdle( void )
{
//Only the player fires this way so we can cast
CBasePlayer *pPlayer = GetOwner()

if ( pPlayer == NULL )
return;

//If we're on a target, play the new anim
if ( pPlayer->IsOnTarget() )
{
SendWeaponAnim( ACT_VM_IDLE_ACTIVE );
}
}
*/
