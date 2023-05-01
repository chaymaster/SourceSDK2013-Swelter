//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basecombatweapon.h"
#include "npcevent.h"
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "player.h"
#include "weapon_ar2.h"
#include "grenade_ar2.h"
#include "gamerules.h"
#include "game.h"
#include "in_buttons.h"
#include "ai_memory.h"
#include "soundent.h"
#include "hl2_player.h"
#include "EntityFlame.h"
#include "weapon_flaregun.h"
#include "te_effect_dispatch.h"
#include "prop_combine_ball.h"
#include "beam_shared.h"
#include "npc_combine.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "basehlcombatweapon_shared.h" //added for simple alt reloading

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar sk_weapon_ar2_alt_fire_radius("sk_weapon_ar2_alt_fire_radius", "10");
ConVar sk_weapon_ar2_alt_fire_duration("sk_weapon_ar2_alt_fire_duration", "2");
ConVar sk_weapon_ar2_alt_fire_mass("sk_weapon_ar2_alt_fire_mass", "150");
extern ConVar    sde_simple_alt_reload;

//=========================================================
//=========================================================

BEGIN_DATADESC(CWeaponAR2)

DEFINE_FIELD(m_flSecondaryReloadActivationTime, FIELD_TIME),
DEFINE_FIELD(m_flSecondaryReloadDeactivationTime, FIELD_TIME),
DEFINE_FIELD(m_flDelayedFire, FIELD_TIME),
DEFINE_FIELD(m_bShotDelayed, FIELD_BOOLEAN),
//DEFINE_FIELD( m_nVentPose, FIELD_INTEGER ),
DEFINE_FIELD(m_flSecondaryEjectTime, FIELD_TIME), //new
DEFINE_FIELD(m_bSecondaryEjectPending, FIELD_BOOLEAN), //new
DEFINE_FIELD(m_flSecondaryEjectTime2, FIELD_TIME), //new
DEFINE_FIELD(m_bSecondaryEjectPending2, FIELD_BOOLEAN), //new

END_DATADESC()

#define BODYGROUP_BULLET1 2
#define BODYGROUP_BULLET2 3

IMPLEMENT_SERVERCLASS_ST(CWeaponAR2, DT_WeaponAR2)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_ar2, CWeaponAR2);
PRECACHE_WEAPON_REGISTER(weapon_ar2);

acttable_t	CWeaponAR2::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_AR2, true },
	{ ACT_RELOAD, ACT_RELOAD_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE, ACT_IDLE_SMG1, true },		// FIXME: hook to AR2 unique
	{ ACT_IDLE_ANGRY, ACT_IDLE_ANGRY_SMG1, true },		// FIXME: hook to AR2 unique

	{ ACT_WALK, ACT_WALK_RIFLE, true },

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
	{ ACT_GESTURE_RANGE_ATTACK1, ACT_GESTURE_RANGE_ATTACK_AR2, false },
	{ ACT_COVER_LOW, ACT_COVER_SMG1_LOW, false },		// FIXME: hook to AR2 unique
	{ ACT_RANGE_AIM_LOW, ACT_RANGE_AIM_AR2_LOW, false },
	{ ACT_RANGE_ATTACK1_LOW, ACT_RANGE_ATTACK_SMG1_LOW, true },		// FIXME: hook to AR2 unique
	{ ACT_RELOAD_LOW, ACT_RELOAD_SMG1_LOW, false },
	{ ACT_GESTURE_RELOAD, ACT_GESTURE_RELOAD_SMG1, true },
	//	{ ACT_RANGE_ATTACK2, ACT_RANGE_ATTACK_AR2_GRENADE, true },
};



IMPLEMENT_ACTTABLE(CWeaponAR2);

CWeaponAR2::CWeaponAR2()
{
	m_fMinRange1 = 65;
	m_fMaxRange1 = 2048;

	m_fMinRange2 = 256;
	m_fMaxRange2 = 1024;

	m_nShotsFired = 0;
	m_nVentPose = -1;

	m_bAltFiresUnderwater = false;
}

void CWeaponAR2::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("prop_combine_ball");
	UTIL_PrecacheOther("env_entity_dissolver");
	PrecacheModel("models/items/combine_rifle_ammo01_off.mdl");
	PrecacheModel("models/items/empty_mag_ar2.mdl");
}

//-----------------------------------------------------------------------------
// Purpose: Handle grenade detonate in-air (even when no ammo is left)
//-----------------------------------------------------------------------------
bool CWeaponAR2::Deploy(void)
{
	m_nShotsFired = 0;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	DisplaySDEHudHint();
	SetSkin(0);
	shouldDropMag = false; //drop mag
	//if (pPlayer)
	//{
	//	CBaseViewModel *pVM = pPlayer->GetViewModel();
	//	if (m_iClip1 == 0)
	//		pVM->SetBodygroup(BODYGROUP_BULLET1, 1);
	//	else
	//		pVM->SetBodygroup(BODYGROUP_BULLET1, 0);
	//
	//	if (m_iClip1 <= 1)
	//		pVM->SetBodygroup(BODYGROUP_BULLET2, 1);
	//	else
	//		pVM->SetBodygroup(BODYGROUP_BULLET2, 0);
	//}
	return BaseClass::Deploy();
}
void CWeaponAR2::ItemPostFrame(void)
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

	if (m_flNextPrimaryAttack <= gpGlobals->curtime + 0.07)
		SetSkin(0);

	// See if we need to fire off our secondary round
	if (m_bShotDelayed && gpGlobals->curtime >= m_flDelayedFire)
	{
		DelayedAttack();
	}
	if (m_bSecondaryEjectPending && gpGlobals->curtime >= m_flSecondaryEjectTime) //new
	{
		SecondaryEject();
	}
	if (m_bSecondaryEjectPending2 && gpGlobals->curtime >= m_flSecondaryEjectTime2) //new
	{
		SecondaryEjectSpawn();
	}

	if (shouldDropMag && (gpGlobals->curtime >= dropMagTime)) //drop mag
	{
		DropMag();
	}

	// Update our pose parameter for the vents
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner)
	{
		CBaseViewModel *pVM = pOwner->GetViewModel();

		if (pVM)
		{
			if (m_nVentPose == -1)
			{
				m_nVentPose = pVM->LookupPoseParameter("VentPoses");
			}

			float flVentPose = RemapValClamped(m_nShotsFired, 0, 5, 0.0f, 1.0f);
			pVM->SetPoseParameter(m_nVentPose, flVentPose);
		}

		//if (m_iClip1 == 0)
		//	pVM->SetBodygroup(BODYGROUP_BULLET1, 1);
		//else
		//	pVM->SetBodygroup(BODYGROUP_BULLET1, 0);
		//
		//if (m_iClip1 <= 1)
		//	pVM->SetBodygroup(BODYGROUP_BULLET2, 1);
		//else
		//	pVM->SetBodygroup(BODYGROUP_BULLET2, 0);
	}

	//DevMsg("AR2:	bullet 1 group id	%d \n", FindBodygroupByName("bullet1"));
	//DevMsg("AR2:	bullet 2 group id	%d \n", FindBodygroupByName("bullet2"));



	BaseClass::ItemPostFrame();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponAR2::GetPrimaryAttackActivity(void)
{
	if (m_bIsIronsighted)
	{
		return ACT_VM_IRONSHOOT;
	}
	else
	{
		if (m_nShotsFired < 2)
			return ACT_VM_PRIMARYATTACK;

		if (m_nShotsFired < 3)
			return ACT_VM_RECOIL1;

		if (m_nShotsFired < 4)
			return ACT_VM_RECOIL2;

		return ACT_VM_RECOIL3;
	}

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &tr - 
//			nDamageType - 
//-----------------------------------------------------------------------------
void CWeaponAR2::DoImpactEffect(trace_t &tr, int nDamageType)
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + (tr.plane.normal * 1.0f);
	data.m_vNormal = tr.plane.normal;

	DispatchEffect("AR2Impact", data);

	BaseClass::DoImpactEffect(tr, nDamageType);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::DelayedAttack(void)
{


	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	CHL2_Player *pHL2Player_Owner = dynamic_cast<CHL2_Player*>(pOwner);

	if (pOwner == NULL)
		return;
	// Deplete the clip completely
	/*if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 1)
	{
	Msg("SDE AR2 reload full \n");
	DisableIronsights();
	SendWeaponAnim(ACT_VM_SECONDARYATTACK_RELOAD);
	m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;
	}
	else
	{
	Msg("SDE AR2 reload empty \n");
	SendWeaponAnim(ACT_VM_SECONDARYATTACK);
	m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	}*/

	Msg("SDE AR2 reload empty \n");
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if ((pPlayer->GetAmmoCount(m_iSecondaryAmmoType) > 1) & (sde_simple_alt_reload.GetInt() == 1))
	{
		DisableIronsights();
		SendWeaponAnim(ACT_VM_SECONDARYATTACK_RELOAD);
		//m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
		//m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;
		m_bSecondaryEjectPending = true;
		m_bSecondaryEjectPending2 = true;
		m_flSecondaryEjectTime = gpGlobals->curtime + 1.5f; //new
		m_flSecondaryEjectTime2 = gpGlobals->curtime + 2.5f; //new
		m_flSecondaryReloadActivationTime = gpGlobals->curtime + 0.1f; // start auto-loading secondary ammo in a small time after secondary fire,
		// forbidding ironsight
	}
	else
	{
		SendWeaponAnim(ACT_VM_SECONDARYATTACK);
		//m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;
		//m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	}

	m_bShotDelayed = false;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flSecondaryReloadDeactivationTime = gpGlobals->curtime + SequenceDuration();
	//can fire primary or reload secondary after animation

	// Register a muzzleflash for the AI
	pOwner->DoMuzzleFlash();
	pOwner->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	WeaponSound(WPN_DOUBLE);

	pOwner->RumbleEffect(RUMBLE_SHOTGUN_DOUBLE, 0, RUMBLE_FLAG_RESTART);

	// Fire the bullets
	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);
	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(vecSrc,
		vecVelocity,
		sk_weapon_ar2_alt_fire_radius.GetFloat(),
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		sk_weapon_ar2_alt_fire_duration.GetFloat(),
		pOwner);

	// View effects
	color32 white = { 255, 255, 255, 64 };
	UTIL_ScreenFade(pOwner, white, 0.1, 0, FFADE_IN);

	//Disorient the player
	QAngle angles = pOwner->GetLocalAngles();

	angles.x += random->RandomInt(-4, 4);
	angles.y += random->RandomInt(-4, 4);
	angles.z = 0;

	pOwner->SnapEyeAngles(angles);

	pOwner->ViewPunch(QAngle(random->RandomInt(-6, -10), random->RandomInt(1, 2), 0));

	// Grenade launcher gets unloaded if alt reload is not simple or when firing last secondary round with auto-reload,
	// to avoid nonsense when you have no rounds but GL is considered loaded
	if (sde_simple_alt_reload.GetInt() == 0 || pPlayer->GetAmmoCount(m_iSecondaryAmmoType) == 1)
		pHL2Player_Owner->AR2_GL_Unload();

	// Decrease ammo
	pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Can shoot again immediately
	//m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

	// Can blow up after a short delay (so have time to release mouse button)
	//m_flNextSecondaryAttack = gpGlobals->curtime + 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::PrimaryAttack(void)
{
	// If my clip is empty (and I use clips) start reload
	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	//pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());
	SetSkin(0);
	// player "shoot" animation
	pPlayer->SetAnimation(PLAYER_ATTACK1);
	pPlayer->DoMuzzleFlash();
	FireBulletsInfo_t info;
	info.m_vecSrc = pPlayer->Weapon_ShootPosition();

	info.m_vecDirShooting = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	// To make the firing framerate independent, we may have to fire more than one bullet here on low-framerate systems, 
	// especially if the weapon we're firing has a really fast rate of fire.
	info.m_iShots = 0;
	float fireRate = GetFireRate();

	while (m_flNextPrimaryAttack <= gpGlobals->curtime)
	{
		// MUST call sound before removing a round from the clip of a CMachineGun
		WeaponSound(SINGLE, m_flNextPrimaryAttack);
		m_flNextPrimaryAttack = m_flNextPrimaryAttack + fireRate;
		info.m_iShots++;
		if (!fireRate)
			break;
	}
	// Make sure we don't fire more than the amount in the clip
	if (UsesClipsForAmmo1())
	{
		info.m_iShots = MIN(info.m_iShots, m_iClip1);
		m_iClip1 -= info.m_iShots;
	}
	else
	{
		info.m_iShots = MIN(info.m_iShots, pPlayer->GetAmmoCount(m_iPrimaryAmmoType));
		pPlayer->RemoveAmmo(info.m_iShots, m_iPrimaryAmmoType);
	}

	info.m_flDistance = MAX_TRACE_LENGTH;
	info.m_iAmmoType = m_iPrimaryAmmoType;
	info.m_iTracerFreq = 2;

#if !defined( CLIENT_DLL )
	// Fire the bullets
	info.m_vecSpread = pPlayer->GetAttackSpread(this);
#else
	//!!!HACKHACK - what does the client want this function for? 
	info.m_vecSpread = GetActiveWeapon()->GetBulletSpread();
#endif // CLIENT_DLL

	pPlayer->FireBullets(info);
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	SetSkin(1);

	//Add our view kick in
	AddViewKick();
}

void CWeaponAR2::SecondaryAttack(void)
{
	if (m_bInReload)
		return; //prevent interruption of primary reload with secondary attack

	if (sde_simple_alt_reload.GetInt() == 0)
	{
		if (m_bShotDelayed)
			return;

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);
		if (pHL2Player->Get_AR2_GLL()) // Grenade launcher loading mechanic when the player wants to - HEVcrab
		{
			// Cannot fire underwater
			if (GetOwner() && GetOwner()->GetWaterLevel() == 3)
			{
				SendWeaponAnim(ACT_VM_DRYFIRE);
				BaseClass::WeaponSound(EMPTY);
				m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
				return;
			}

			m_bShotDelayed = true;
			m_flDelayedFire = gpGlobals->curtime + 0.5f;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 0.6f;

			SetSkin(0);
			// m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + 2.7f;
			// m_flNextPrimaryAttack = gpGlobals->curtime + 2.7f; // m_flNextSecondaryAttack is set in DelayedAttack()

			CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
			if (pPlayer)
			{
				pPlayer->RumbleEffect(RUMBLE_AR2_ALT_FIRE, 0, RUMBLE_FLAG_RESTART);
			}

			SendWeaponAnim(ACT_VM_FIDGET);
			WeaponSound(SPECIAL1);

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
			m_bSecondaryEjectPending = true; //new
			m_bSecondaryEjectPending2 = true; //new
			m_flSecondaryEjectTime = gpGlobals->curtime + 1.0f; //new
			m_flSecondaryEjectTime2 = gpGlobals->curtime + 2.0f; //new
			pHL2Player->AR2_GL_Load();
			pHL2Player->ShowCrosshair(true); //for the case of reloading grenade launcher when in toggle ironsight
			//engine->ClientCommand(edict(), "testhudanim %s", "AmmoSecondaryIncreased");

			//secondary_ammo_recolor_crutch = true;
		}
	}
	else
	{
		if (m_bShotDelayed)
			return;

		// Cannot fire underwater
		if (GetOwner() && GetOwner()->GetWaterLevel() == 3)
		{
			SendWeaponAnim(ACT_VM_DRYFIRE);
			BaseClass::WeaponSound(EMPTY);
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
			return;
		}

		m_bShotDelayed = true;
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;

		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer)
		{
			pPlayer->RumbleEffect(RUMBLE_AR2_ALT_FIRE, 0, RUMBLE_FLAG_RESTART);
		}

		SendWeaponAnim(ACT_VM_FIDGET);
		WeaponSound(SPECIAL1);

		m_iSecondaryAttacks++;
		gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
	}
}

/* стандартный код подствола
void CWeaponAR2::SecondaryAttack(void)
{
if (m_bShotDelayed)
return;

// Cannot fire underwater
if (GetOwner() && GetOwner()->GetWaterLevel() == 3)
{
SendWeaponAnim(ACT_VM_DRYFIRE);
BaseClass::WeaponSound(EMPTY);
m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
return;
}

m_bShotDelayed = true;
m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;

CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
if (pPlayer)
{
pPlayer->RumbleEffect(RUMBLE_AR2_ALT_FIRE, 0, RUMBLE_FLAG_RESTART);
}

SendWeaponAnim(ACT_VM_FIDGET);
WeaponSound(SPECIAL1);

m_iSecondaryAttacks++;
gamestats->Event_WeaponFired(pPlayer, false, GetClassname());
}
*/

/* AR1M1 underbarrel reload
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeaponAR2::SecondaryAttack(void)
{
// Only the player fires this way so we can cast
CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

if (pPlayer == NULL)
return;

CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player*>(pPlayer);

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
m_flNextPrimaryAttack = gpGlobals->curtime + 2.2f;
m_flNextSecondaryAttack = gpGlobals->curtime + 2.2f;
pHL2Player->AR1M1_GL_Load();
pHL2Player->ShowCrosshair(true); //for the case of reloading grenade launcher when in toggle ironsight
//engine->ClientCommand(edict(), "testhudanim %s", "AmmoSecondaryIncreased");

//secondary_ammo_recolor_crutch = true;
}

}
*/

void CWeaponAR2::HoldIronsight(void)
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
// Purpose: Override if we're waiting to release a shot
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponAR2::CanHolster(void)
{
	if (m_bShotDelayed)
		return false;

	return BaseClass::CanHolster();
}

//-----------------------------------------------------------------------------
// Purpose: Override if we're waiting to release a shot
//-----------------------------------------------------------------------------
//bool CWeaponAR2::Reload(void) старая перезарядка
//{
//	if (m_bShotDelayed)
//		return false;
//
//	return BaseClass::Reload();
//}

bool CWeaponAR2::Reload(void)
{
	if (m_bInSecondaryReload)
		return false; //prevent interruption of secondary reload with primary reload

	float fCacheTime = m_flNextSecondaryAttack;

	SetSkin(0);
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		//if (pPlayer)
		//{
		//	CBaseViewModel *pVM = pPlayer->GetViewModel();
		//	pVM->SetBodygroup(BODYGROUP_BULLET1, 1);
		//	pVM->SetBodygroup(BODYGROUP_BULLET2, 1);
		//}
		pPlayer->ShowCrosshair(true); // show crosshair to fix crosshair for reloading weapons in toggle ironsight
		if (m_iClip1 < 1)
		{
			bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
			if (fRet)
			{
				WeaponSound(RELOAD);
				m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				dropMagTime = (gpGlobals->curtime + 0.7f); //drop mag
				shouldDropMag = true; //drop mag
			}
			return fRet;
		}
		else
		{
			bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_NOBOLD);
			if (fRet)
			{
				WeaponSound(RELOAD);
				m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				dropMagTime = (gpGlobals->curtime + 0.7f); //drop mag
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
// Input  : *pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	Vector vecShootOrigin, vecShootDir;

	CAI_BaseNPC *npc = pOperator->MyNPCPointer();
	ASSERT(npc != NULL);

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

	WeaponSoundRealtime(SINGLE_NPC);

	CSoundEnt::InsertSound(SOUND_COMBAT | SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy());

	pOperator->FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2);

	// NOTENOTE: This is overriden on the client-side
	// pOperator->DoMuzzleFlash();

	m_iClip1 = m_iClip1 - 1;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles)
{
	WeaponSound(WPN_DOUBLE);

	if (!GetOwner())
		return;

	CAI_BaseNPC *pNPC = GetOwner()->MyNPCPointer();
	if (!pNPC)
		return;

	// Fire!
	Vector vecSrc;
	Vector vecAiming;

	if (bUseWeaponAngles)
	{
		QAngle	angShootDir;
		GetAttachment(LookupAttachment("muzzle"), vecSrc, angShootDir);
		AngleVectors(angShootDir, &vecAiming);
	}
	else
	{
		vecSrc = pNPC->Weapon_ShootPosition();

		Vector vecTarget;

		CNPC_Combine *pSoldier = dynamic_cast<CNPC_Combine *>(pNPC);
		if (pSoldier)
		{
			// In the distant misty past, elite soldiers tried to use bank shots.
			// Therefore, we must ask them specifically what direction they are shooting.
			vecTarget = pSoldier->GetAltFireTarget();
		}
		else
		{
			// All other users of the AR2 alt-fire shoot directly at their enemy.
			if (!pNPC->GetEnemy())
				return;

			vecTarget = pNPC->GetEnemy()->BodyTarget(vecSrc);
		}

		vecAiming = vecTarget - vecSrc;
		VectorNormalize(vecAiming);
	}

	Vector impactPoint = vecSrc + (vecAiming * MAX_TRACE_LENGTH);

	float flAmmoRatio = 1.0f;
	float flDuration = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 0.5f, sk_weapon_ar2_alt_fire_duration.GetFloat());
	float flRadius = RemapValClamped(flAmmoRatio, 0.0f, 1.0f, 4.0f, sk_weapon_ar2_alt_fire_radius.GetFloat());

	// Fire the bullets
	Vector vecVelocity = vecAiming * 1000.0f;

	// Fire the combine ball
	CreateCombineBall(vecSrc,
		vecVelocity,
		flRadius,
		sk_weapon_ar2_alt_fire_mass.GetFloat(),
		flDuration,
		pNPC);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_ForceNPCFire(CBaseCombatCharacter *pOperator, bool bSecondary)
{
	if (bSecondary)
	{
		FireNPCSecondaryAttack(pOperator, true);
	}
	else
	{
		// Ensure we have enough rounds in the clip
		m_iClip1++;

		FireNPCPrimaryAttack(pOperator, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CWeaponAR2::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_AR2:
	{
							 FireNPCPrimaryAttack(pOperator, false);
	}
		break;

	case EVENT_WEAPON_AR2_ALTFIRE:
	{
									 FireNPCSecondaryAttack(pOperator, false);
	}
		break;

	default:
		CBaseCombatWeapon::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponAR2::AddViewKick(void)
{
#define	EASY_DAMPEN			0.5f
#define	MAX_VERTICAL_KICK	10.0f	//Degrees
#define	SLIDE_LIMIT			5.0f	//Seconds

	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
		return;

	float flDuration = m_fFireDuration;

	if (g_pGameRules->GetAutoAimMode() == AUTOAIM_ON_CONSOLE)
	{
		// On the 360 (or in any configuration using the 360 aiming scheme), don't let the
		// AR2 progressive into the late, highly inaccurate stages of its kick. Just
		// spoof the time to make it look (to the kicking code) like we haven't been
		// firing for very long.
		flDuration = MIN(flDuration, 0.75f);
	}

	DoMachineGunKick(pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, flDuration, SLIDE_LIMIT);
}

//-----------------------------------------------------------------------------
const WeaponProficiencyInfo_t *CWeaponAR2::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0, 0.75 },
		{ 5.00, 0.75 },
		{ 3.0, 0.85 },
		{ 5.0 / 3.0, 0.75 },
		{ 1.00, 1.0 },
	};

	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}

void CWeaponAR2::SetSkin(int skinNum)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	pViewModel->m_nSkin = skinNum;
}

void CWeaponAR2::SecondaryEject(void)
{
	m_bSecondaryEjectPending = false;
	SetSkin(2);
}

void CWeaponAR2::SecondaryEjectSpawn(void)
{
	m_bSecondaryEjectPending2 = false;
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
	{
		Vector SpawnHeight(0, 0, 36); // высота спауна энергосферного контейнера
		QAngle ForwardAngles = pPlayer->EyeAngles(); // + pPlayer->GetPunchAngle() математически неправильно так просто прибавлять, да и смысл?
		Vector vecForward, vecRight, vecUp;
		AngleVectors(ForwardAngles, &vecForward, &vecRight, &vecUp);
		Vector vecEject = SpawnHeight + 10 * vecForward + 30 * vecRight;

		CBaseEntity *pEjectProp = (CBaseEntity *)CreateEntityByName("prop_physics_override");

		if (pEjectProp)
		{
			// Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 32 + Vector(0, -8, 16);
			// Vector vecOrigin = pPlayer->GetAbsOrigin() + Vector(-16, -48, 16);
			Vector vecOrigin = pPlayer->GetAbsOrigin() + vecEject;
			QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 0.5, 0);
			pEjectProp->SetAbsOrigin(vecOrigin);
			pEjectProp->SetAbsAngles(vecAngles);
			pEjectProp->KeyValue("model", "models/items/combine_rifle_ammo01_off.mdl");
			pEjectProp->KeyValue("solid", "1");
			pEjectProp->KeyValue("targetname", "EjectProp");
			pEjectProp->KeyValue("spawnflags", "260");
			pEjectProp->SetAbsVelocity(vecForward);
			DispatchSpawn(pEjectProp);
			pEjectProp->Activate();
			pEjectProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pEjectProp->SUB_StartFadeOut(30, false);
		}
	}
}

void CWeaponAR2::DropMag(void) //drop mag
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
			pEjectProp->KeyValue("model", "models/items/empty_mag_ar2.mdl");
			pEjectProp->KeyValue("solid", "1");
			pEjectProp->KeyValue("targetname", "EjectProp");
			pEjectProp->KeyValue("spawnflags", "260");
			pEjectProp->SetAbsVelocity(vecForward);
			DispatchSpawn(pEjectProp);
			pEjectProp->Activate();
			pEjectProp->Teleport(&vecOrigin, &vecAngles, NULL);
			pEjectProp->SUB_StartFadeOut(15, false);
		}
	}
}