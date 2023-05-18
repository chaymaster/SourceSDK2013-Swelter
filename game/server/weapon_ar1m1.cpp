//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "game.h"
#include "in_buttons.h"
#include "grenade_ar2.h"
#include "ai_memory.h"
#include "soundent.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include <player.h>
#include <hl2_player.h>
#include "basehlcombatweapon_shared.h" //added for simple alt reloading

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar    sk_plr_dmg_smg1_grenade;
extern ConVar    sde_simple_alt_reload;
extern ConVar	sde_drop_mag;

class CWeaponar1m1 : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponar1m1, CHLSelectFireMachineGun);


	CWeaponar1m1();

	DECLARE_SERVERCLASS();

	void	Precache(void);
	void	AddViewKick(void);
	void	SecondaryAttack(void);
	void	HoldIronsight(void);
	void	PrimaryAttack(void);
	void	ItemPostFrame(void);
	bool	Deploy(void);

	int		GetMinBurst() { return 1; }
	int		GetMaxBurst() { return 3; }

	virtual void Equip(CBaseCombatCharacter *pOwner);
	bool	Reload(void);

	float	GetFireRate(void) { return 0.095f; }	// 13.3hz
	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition(float flDot, float flDist);
	Activity	GetPrimaryAttackActivity(void);

	bool shouldDropMag; //drop mag
	float dropMagTime; //drop mag
	void DropMag(void); //drop mag

	virtual const Vector& GetBulletSpread(void)
	{
		{
			if (m_bIsIronsighted)
			{
				static const Vector cone = VECTOR_CONE_1DEGREES;
				static const Vector injuredCone = VECTOR_CONE_1DEGREES;
				return cone;
			}
			else
			{
				static const Vector cone = VECTOR_CONE_5DEGREES;
				static const Vector injuredCone = VECTOR_CONE_5DEGREES;
				return cone;
			}
		}
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir);
	void Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
	float m_flSecondaryReloadActivationTime; //new
	float m_flSecondaryReloadDeactivationTime; //new
};

IMPLEMENT_SERVERCLASS_ST(CWeaponar1m1, DT_Weaponar1m1)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_ar1m1, CWeaponar1m1);
PRECACHE_WEAPON_REGISTER(weapon_ar1m1);

BEGIN_DATADESC(CWeaponar1m1)

DEFINE_FIELD(m_vecTossVelocity, FIELD_VECTOR),
DEFINE_FIELD(m_flNextGrenadeCheck, FIELD_TIME),
DEFINE_FIELD(m_flSecondaryReloadActivationTime, FIELD_TIME),
DEFINE_FIELD(m_flSecondaryReloadDeactivationTime, FIELD_TIME),

END_DATADESC()

acttable_t	CWeaponar1m1::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SMG1, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },
	{ ACT_IDLE, ACT_IDLE_SMG1, true },
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },

	{ ACT_WALK, ACT_WALK_RIFLE, true },
	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },

	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED, ACT_IDLE_SMG1_RELAXED, false },//never aims
	{ ACT_IDLE_STIMULATED, ACT_IDLE_SMG1_STIMULATED, false },
	{ ACT_IDLE_AGITATED, ACT_IDLE_ANGRY_SMG1, false },//always aims

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

	{ ACT_WALK_AIM, ACT_WALK_AIM_RIFLE, true },
	{ ACT_WALK_CROUCH, ACT_WALK_CROUCH_RIFLE, true },
	{ ACT_WALK_CROUCH_AIM, ACT_WALK_CROUCH_AIM_RIFLE, true },
	{ ACT_RUN, ACT_RUN_RIFLE, true },
	{ ACT_RUN_AIM, ACT_RUN_AIM_RIFLE, true },
	{ ACT_RUN_CROUCH, ACT_RUN_CROUCH_RIFLE, true },
	{ ACT_RUN_CROUCH_AIM, ACT_RUN_CROUCH_AIM_RIFLE, true },
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_SMG1, true },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_SMG1_LOW, false },
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
};

IMPLEMENT_ACTTABLE(CWeaponar1m1);

//=========================================================
CWeaponar1m1::CWeaponar1m1()
{
	m_fMinRange1 = 0;// No minimum range. 
	m_fMaxRange1 = 1400;

	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::Precache(void)
{
	UTIL_PrecacheOther("grenade_ar2");
	PrecacheModel("models/items/empty_mag_ak.mdl");
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponar1m1::Equip(CBaseCombatCharacter *pOwner)
{
	if (pOwner->Classify() == CLASS_PLAYER_ALLY)
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip(pOwner);
}
bool CWeaponar1m1::Deploy(void)
{
	m_nShotsFired = 0;
	Msg("SDE_SMG!_deploy\n");
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	DisplaySDEHudHint();
	shouldDropMag = false;
	return BaseClass::Deploy();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir)
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());
	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0);

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment(LookupAttachment("muzzle"), vecShootOrigin, angShootDir);
	AngleVectors(angShootDir, &vecShootDir);
	FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_SMG1:
	{
		Vector vecShootOrigin, vecShootDir;
		QAngle angDiscard;

		// Support old style attachment point firing
		if ((pEvent->options == NULL) || (pEvent->options[0] == '\0') || (!pOperator->GetAttachment(pEvent->options, vecShootOrigin, angDiscard)))
		{
			vecShootOrigin = pOperator->Weapon_ShootPosition();
		}

		CAI_BaseNPC *npc = pOperator->MyNPCPointer();
		ASSERT(npc != NULL);
		vecShootDir = npc->GetActualShootTrajectory(vecShootOrigin);

		FireNPCPrimaryAttack(pOperator, vecShootOrigin, vecShootDir);
	}
	break;

	/*//FIXME: Re-enable
	case EVENT_WEAPON_AR2_GRENADE:
	{
	CAI_BaseNPC *npc = pOperator->MyNPCPointer();

	Vector vecShootOrigin, vecShootDir;
	vecShootOrigin = pOperator->Weapon_ShootPosition();
	vecShootDir = npc->GetShootEnemyDir( vecShootOrigin );

	Vector vecThrow = m_vecTossVelocity;

	CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create( "grenade_ar2", vecShootOrigin, vec3_angle, npc );
	pGrenade->SetAbsVelocity( vecThrow );
	pGrenade->SetLocalAngularVelocity( QAngle( 0, 400, 0 ) );
	pGrenade->SetMoveType( MOVETYPE_FLYGRAVITY );
	pGrenade->m_hOwner			= npc;
	pGrenade->m_pMyWeaponAR2	= this;
	pGrenade->SetDamage(sk_npc_dmg_ar2_grenade.GetFloat());

	// FIXME: arrgg ,this is hard coded into the weapon???
	m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

	m_iClip2--;
	}
	break;
	*/

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponar1m1::GetPrimaryAttackActivity(void)
{
	if (m_nShotsFired < 2)
		return ACT_VM_PRIMARYATTACK;

	if (m_nShotsFired < 3)
		return ACT_VM_RECOIL1;

	if (m_nShotsFired < 4)
		return ACT_VM_RECOIL2;

	return ACT_VM_RECOIL3;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponar1m1::Reload(void)
{
	if (m_bInSecondaryReload)
		return false; //prevent interruption of secondary reload with primary reload

	float fCacheTime = m_flNextSecondaryAttack;


	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		pPlayer->ShowCrosshair(true); // show crosshair to fix crosshair for reloading weapons in toggle ironsight
		if (m_iClip1 < 1)
		{
			Msg("SDE_R+ \n");
			bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_NOBOLD);
			if (fRet)
			{
				WeaponSound(RELOAD);
				m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				dropMagTime = (gpGlobals->curtime + 0.7f); //drop mag
				if (sde_drop_mag.GetInt())
					shouldDropMag = true; //drop mag
			}
			return fRet;
		}
		else
		{
			Msg("SDE_R- \n");
			bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
			if (fRet)
			{
				WeaponSound(RELOAD);
				m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				dropMagTime = (gpGlobals->curtime + 0.7f); //drop mag
				if (sde_drop_mag.GetInt())
					shouldDropMag = true; //drop mag
			}
			return fRet;
		}
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	4.0f	//Degrees
#define	SLIDE_LIMIT			8.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponar1m1::SecondaryAttack(void)
{
	if (m_bInReload)
		return; //prevent interruption of primary reload with secondary attack

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

	if (sde_simple_alt_reload.GetInt() == 0)
	{

		if (pHL2Player->Get_AR1M1_GLL()) // Grenade launcher loading mechanic when the player wants to - HEVcrab
		{

			//Must have ammo
			if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0) || (pPlayer->GetWaterLevel() == 3))
			{
				SendWeaponAnim(ACT_VM_DRYFIRE);
				BaseClass::WeaponSound(EMPTY);
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
				return;
			}

			if (m_bInReload)
				m_bInReload = false;

			// MUST call sound before removing a round from the clip of a CMachineGun
			BaseClass::WeaponSound(WPN_DOUBLE);

			pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

			Vector vecSrc = pPlayer->Weapon_ShootPosition();
			Vector	vecThrow;
			// Don't autoaim on grenade tosses
			AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
			VectorScale(vecThrow, 2000.0f, vecThrow);

			//Create the grenade
			QAngle angles;
			VectorAngles(vecThrow, angles);
			CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
			pGrenade->SetAbsVelocity(vecThrow);

			//pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
			pGrenade->SetLocalAngularVelocity(RandomAngle(-5, 5));
			pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
			pGrenade->SetThrower(GetOwner());
			pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

			//WeaponSound(RELOAD);
			CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

			// player "shoot" animation
			//pPlayer->SetAnimation( PLAYER_ATTACK1 );

			//HERE ENABLE ONLY ONE PART OF THE CYCLE WHICH CORRESPONDS TO FALSE BRANCH - HEVcrab

			/*if (pPlayer->GetAmmoCount(m_iSecondaryAmmoType) > 1)
			{

			DisableIronsights();
			SendWeaponAnim(ACT_VM_SECONDARYATTACK_RELOAD);
			m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
			m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;

			}*/
			//else
			//{
			SendWeaponAnim(ACT_VM_SECONDARYATTACK);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
			//}


			// Grenade launcher gets unloaded
			pHL2Player->AR1M1_GL_Unload();
			//engine->ClientCommand(edict(), "testhudanim %s", "AmmoSecondaryDecreasedUnloaded");

			// Can shoot again immediately
			// Decrease ammo
			pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

			// Register a muzzleflash for the AI.
			pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

			m_iSecondaryAttacks++;
			gamestats->Event_WeaponFired(pPlayer, false, GetClassname());

		}

		else if (pPlayer->GetAmmoCount(m_iSecondaryAmmoType) > 0) // If the grenade launcher is not loaded, but player has ammo for it, load it - HEVcrab
		{
			DisableIronsights();
			SendWeaponAnim(ACT_VM_SECONDARY_RELOAD);
			//m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
			//m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;
			m_flSecondaryReloadActivationTime = gpGlobals->curtime; // signal the secondary reload to ItemPostFrame() immediately to forbid ironsight
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flSecondaryReloadDeactivationTime = gpGlobals->curtime + SequenceDuration();
			pHL2Player->AR1M1_GL_Load();
			pHL2Player->ShowCrosshair(true); //for the case of reloading grenade launcher when in toggle ironsight
			//engine->ClientCommand(edict(), "testhudanim %s", "AmmoSecondaryIncreased");

			//secondary_ammo_recolor_crutch = true;
		}
	}
	else
	{

		//Must have ammo
		if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) <= 0) || (pPlayer->GetWaterLevel() == 3))
		{
			SendWeaponAnim(ACT_VM_DRYFIRE);
			BaseClass::WeaponSound(EMPTY);
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
			return;
		}

		if (m_bInReload)
			m_bInReload = false;

		// MUST call sound before removing a round from the clip of a CMachineGun
		BaseClass::WeaponSound(WPN_DOUBLE);

		pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

		Vector vecSrc = pPlayer->Weapon_ShootPosition();
		Vector	vecThrow;
		// Don't autoaim on grenade tosses
		AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
		VectorScale(vecThrow, 2000.0f, vecThrow);

		//Create the grenade
		QAngle angles;
		VectorAngles(vecThrow, angles);
		CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
		pGrenade->SetAbsVelocity(vecThrow);

		//pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
		pGrenade->SetLocalAngularVelocity(RandomAngle(-5, 5));
		pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
		pGrenade->SetThrower(GetOwner());
		pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

		//WeaponSound(RELOAD);
		CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

		// player "shoot" animation
		//pPlayer->SetAnimation( PLAYER_ATTACK1 );
		if (pPlayer->GetAmmoCount(m_iSecondaryAmmoType) > 1)
		{
			DisableIronsights();
			SendWeaponAnim(ACT_VM_SECONDARYATTACK_RELOAD);
			//m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
			//m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;
			m_flSecondaryReloadActivationTime = gpGlobals->curtime + 0.1f; // start auto-loading secondary ammo in a small time after secondary fire,
			// forbidding ironsight
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flSecondaryReloadDeactivationTime = gpGlobals->curtime + SequenceDuration();
		}
		else
		{
			SendWeaponAnim(ACT_VM_SECONDARYATTACK);
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
			// Grenade launcher gets unloaded when firing last secondary round with auto-reload,
			// to avoid nonsense when you have no rounds but GL is considered loaded
			if (pPlayer->GetAmmoCount(m_iSecondaryAmmoType) == 1)
				pHL2Player->AR1M1_GL_Unload();
		}

		// Decrease ammo
		pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

		// Can shoot again immediately


		// Register a muzzleflash for the AI.
		pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

		m_iSecondaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
	}

}

void CWeaponar1m1::HoldIronsight(void)
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

void CWeaponar1m1::ItemPostFrame(void)
{
	if (gpGlobals->curtime >= m_flSecondaryReloadActivationTime)
	{
		m_bInSecondaryReload = true;
	}

	if (gpGlobals->curtime >= m_flSecondaryReloadDeactivationTime)
	{
		m_bInSecondaryReload = false;
	}

	// forbid ironsight if secondary reload has been activated but non deactivated yet

	// Ironsight if not reloading
	if (!(m_bInReload || m_bInSecondaryReload))
		HoldIronsight();

	if (shouldDropMag && (gpGlobals->curtime > dropMagTime)) //drop mag
	{
		DropMag();
	}

	BaseClass::ItemPostFrame();
}

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponar1m1::WeaponRangeAttack2Condition(float flDot, float flDist)
{
	CAI_BaseNPC *npcOwner = GetOwner()->MyNPCPointer();

	return COND_NONE;

	/*
	// --------------------------------------------------------
	// Assume things haven't changed too much since last time
	// --------------------------------------------------------
	if (gpGlobals->curtime < m_flNextGrenadeCheck )
	return m_lastGrenadeCondition;
	*/

	// -----------------------
	// If moving, don't check.
	// -----------------------
	if (npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if (!(pEnemy->GetFlags() & FL_ONGROUND) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z))
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		return COND_NONE;
	}

	// --------------------------------------
	//  Get target vector
	// --------------------------------------
	Vector vecTarget;
	if (random->RandomInt(0, 1))
	{
		// magically know where they are
		vecTarget = pEnemy->WorldSpaceCenter();
	}
	else
	{
		// toss it to where you last saw them
		vecTarget = vecEnemyLKP;
	}
	// vecTarget = m_vecEnemyLKP + (pEnemy->BodyTarget( GetLocalOrigin() ) - pEnemy->GetLocalOrigin());
	// estimate position
	// vecTarget = vecTarget + pEnemy->m_vecVelocity * 2;


	if ((vecTarget - npcOwner->GetLocalOrigin()).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return (COND_NONE);
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ((pTarget = gEntList.FindEntityInSphere(pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST)) != NULL)
	{
		//Check to see if the default relationship is hatred, and if so intensify that
		if (npcOwner->IRelationType(pTarget) == D_LI)
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
			return (COND_WEAPON_BLOCKED_BY_FRIEND);
		}
	}

	// ---------------------------------------------------------------------
	// Check that throw is legal and clear
	// ---------------------------------------------------------------------
	// FIXME: speed is based on difficulty...

	Vector vecToss = VecCheckThrow(this, npcOwner->GetLocalOrigin() + Vector(0, 0, 60), vecTarget, 600.0, 0.5);
	if (vecToss != vec3_origin)
	{
		m_vecTossVelocity = vecToss;

		// don't check again for a while.
		// JAY: HL1 keeps checking - test?
		//m_flNextGrenadeCheck = gpGlobals->curtime;
		m_flNextGrenadeCheck = gpGlobals->curtime + 0.3; // 1/3 second.
		return COND_CAN_RANGE_ATTACK2;
	}
	else
	{
		// don't check again for a while.
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return COND_WEAPON_SIGHT_OCCLUDED;
	}
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponar1m1::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 10.0 / 3.0, 0.75 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}
void CWeaponar1m1::PrimaryAttack(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;

	if ((UsesClipsForAmmo1() && m_iClip1 == 0) || (!UsesClipsForAmmo1() && !pPlayer->GetAmmoCount(m_iPrimaryAmmoType)))
		return;

	m_nShotsFired++;

	pPlayer->DoMuzzleFlash();

	int iBulletsToFire = 0;

	float fireRate = 0.0;
	QAngle	viewPunch;

	if (m_bIsIronsighted)
	{
		fireRate = 0.11f;
		SendWeaponAnim(ACT_VM_IRONSHOOT);

		viewPunch.x = random->RandomFloat(0.3f, 0.7f);
		viewPunch.y = random->RandomFloat(-0.7f, 0.7f);
		viewPunch.z = 0.0f;
	}
	else
	{
		fireRate = 0.099f;
		SendWeaponAnim(ACT_VM_PRIMARYATTACK);

		viewPunch.x = random->RandomFloat(0.7f, 1.0f);
		viewPunch.y = random->RandomFloat(-1.2f, 1.2f);
		viewPunch.z = 0.0f;
	}

	// MUST call sound before removing a round from the clip of a CHLMachineGun
	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		iBulletsToFire++;
	}

	// Make sure we don't fire more than the amount in the clip, if this weapon uses clips
	if (UsesClipsForAmmo1())
	{
		if (iBulletsToFire > m_iClip1)
			iBulletsToFire = m_iClip1;
		m_iClip1 -= iBulletsToFire;
	}

	m_iPrimaryAttacks++;
	//gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	// Fire the bullets
	FireBulletsInfo_t info;
	info.m_iShots = iBulletsToFire;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();
	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;
	FireBullets(info);


	pPlayer->ViewPunch(viewPunch);

	//Factor in the view kick
	AddViewKick();

	CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pPlayer);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	// Register a muzzleflash for the AI
	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);
}

void CWeaponar1m1::DropMag(void) //drop mag
{
	shouldDropMag = false;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		Vector SpawnHeight(0, 0, 50); // высота спауна энергосферного контейнера
		QAngle ForwardAngles = pPlayer->EyeAngles(); // + pPlayer->GetPunchAngle() математически неправильно так просто прибавлять, да и смысл?
		Vector vecForward, vecRight, vecUp;
		AngleVectors(ForwardAngles, &vecForward, &vecRight, &vecUp);
		Vector vecEject = SpawnHeight + 10 * vecRight - 20 * vecUp;

		CBaseEntity *pEjectProp = (CBaseEntity *)CreateEntityByName("prop_physics_override");

		if (pEjectProp)
		{
			Vector vecOrigin = pPlayer->GetAbsOrigin() + vecEject;
			QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 0.5, 0);
			pEjectProp->SetAbsOrigin(vecOrigin);
			pEjectProp->SetAbsAngles(vecAngles);
			pEjectProp->KeyValue("model", "models/items/empty_mag_ak.mdl");
			pEjectProp->KeyValue("solid", "1");
			pEjectProp->KeyValue("targetname", "EjectProp");
			pEjectProp->KeyValue("spawnflags", "516");
			pEjectProp->SetAbsVelocity(vecForward);
			DispatchSpawn(pEjectProp);
			pEjectProp->Activate();
			pEjectProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pEjectProp->SUB_StartFadeOut(30, false);
		}
	}
}