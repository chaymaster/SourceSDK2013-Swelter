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
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar    sk_plr_dmg_smg1_grenade;	

class CWeaponGlauncher : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponGlauncher, CHLSelectFireMachineGun );

	CWeaponGlauncher();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );
	void	AddViewKick( void );
	void	PrimaryAttack(void);
	void	SecondaryAttack( void );
	void	ItemPostFrame( void );
	void	SetSkin(int skinNum);
	bool Deploy(void);

	int		GetMinBurst() { return 2; }
	int		GetMaxBurst() { return 5; }

	virtual void Equip( CBaseCombatCharacter *pOwner );
	bool	Reload( void );

	float	GetFireRate( void ) { return 1.0f; }	// 13.3hz
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition( float flDot, float flDist );
	Activity	GetPrimaryAttackActivity( void );

	virtual const Vector& GetBulletSpread( void )
	{
		if (m_bIsIronsighted)
		{
			static const Vector cone = VECTOR_CONE_3DEGREES;
			return cone;
		}
		else
		{
			static const Vector cone = VECTOR_CONE_6DEGREES;
			return cone;
		}
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

	void FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir );
	void Operator_ForceNPCFire( CBaseCombatCharacter  *pOperator, bool bSecondary );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
	void PickupAnim(void);
	public: int smg1_anim_status = 0;
	DECLARE_ACTTABLE();

protected:

	Vector	m_vecTossVelocity;
	float	m_flNextGrenadeCheck;
};

IMPLEMENT_SERVERCLASS_ST(CWeaponGlauncher, DT_WeaponGlauncher)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_Glauncher, CWeaponGlauncher );
PRECACHE_WEAPON_REGISTER(weapon_Glauncher);

BEGIN_DATADESC( CWeaponGlauncher )

	DEFINE_FIELD( m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( m_flNextGrenadeCheck, FIELD_TIME ),

END_DATADESC()

acttable_t	CWeaponGlauncher::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_SMG1,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },

	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true  },
	
// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims

// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
//End readiness activities

	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_SMG1,	true },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_SMG1_LOW,			false },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
};

IMPLEMENT_ACTTABLE(CWeaponGlauncher);

//=========================================================
CWeaponGlauncher::CWeaponGlauncher( )
{
	m_fMinRange1		= 0;// No minimum range. 
	m_fMaxRange1		= 1400;

	m_bAltFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::Precache( void )
{
	UTIL_PrecacheOther("grenade_ar2");

	BaseClass::Precache();
}
bool CWeaponGlauncher::Deploy(void)
{

	Msg("SDE_SMG!_deploy\n");
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (pPlayer)
		pPlayer->ShowCrosshair(true);
	return BaseClass::Deploy();
}
//-----------------------------------------------------------------------------
// Purpose: Give this weapon longer range when wielded by an ally NPC.
//-----------------------------------------------------------------------------
void CWeaponGlauncher::Equip( CBaseCombatCharacter *pOwner )
{
	if( pOwner->Classify() == CLASS_PLAYER_ALLY )
	{
		m_fMaxRange1 = 3000;
	}
	else
	{
		m_fMaxRange1 = 1400;
	}

	BaseClass::Equip( pOwner );
}

void CWeaponGlauncher::PickupAnim(void)
{
	SendWeaponAnim(ACT_VM_DRYFIRE);
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::FireNPCPrimaryAttack( CBaseCombatCharacter *pOperator, Vector &vecShootOrigin, Vector &vecShootDir )
{
	// FIXME: use the returned number of bullets to account for >10hz firerate
	WeaponSoundRealtime( SINGLE_NPC );

	CSoundEnt::InsertSound( SOUND_COMBAT|SOUND_CONTEXT_GUNFIRE, pOperator->GetAbsOrigin(), SOUNDENT_VOLUME_MACHINEGUN, 0.2, pOperator, SOUNDENT_CHANNEL_WEAPON, pOperator->GetEnemy() );
	pOperator->FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_PRECALCULATED,
		MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 2, entindex(), 0 );

	pOperator->DoMuzzleFlash();
	m_iClip1 = m_iClip1 - 1;
}
void CWeaponGlauncher::SetSkin(int skinNum)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	CBaseViewModel *pViewModel = pOwner->GetViewModel();

	if (pViewModel == NULL)
		return;

	pViewModel->m_nSkin = skinNum;
}
void CWeaponGlauncher::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	// Debounce the recoiling counter
	if ((pOwner->m_nButtons & IN_ATTACK) == false)
	{
		m_nShotsFired = 0;
	}

	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
		return;
	if (pPlayer)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		// подготавливаем вектора и трасировщики
		trace_t tr;
		Vector	vecStart, vecStop, vecDir;

		// получаем углы
		AngleVectors(pPlayer->EyeAngles(), &vecDir);

		// получаем вектора
		vecStart = pPlayer->EyePosition();
		vecStop = vecStart + vecDir * MAX_TRACE_LENGTH;


	}

	BaseClass::ItemPostFrame();
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::Operator_ForceNPCFire( CBaseCombatCharacter *pOperator, bool bSecondary )
{
	// Ensure we have enough rounds in the clip
	m_iClip1++;

	Vector vecShootOrigin, vecShootDir;
	QAngle	angShootDir;
	GetAttachment( LookupAttachment( "muzzle" ), vecShootOrigin, angShootDir );
	AngleVectors( angShootDir, &vecShootDir );
	FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
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
			ASSERT( npc != NULL );
			vecShootDir = npc->GetActualShootTrajectory( vecShootOrigin );

			FireNPCPrimaryAttack( pOperator, vecShootOrigin, vecShootDir );
		}
	case ACT_RANGE_ATTACK_SHOTGUN:
		{
			CAI_BaseNPC *npc = pOperator->MyNPCPointer();

			Vector vecShootOrigin, vecShootDir;
			vecShootOrigin = pOperator->Weapon_ShootPosition();
			vecShootDir = npc->GetShootEnemyDir(vecShootOrigin);

			Vector vecThrow = m_vecTossVelocity;

			CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecShootOrigin, vec3_angle, npc);
			pGrenade->SetAbsVelocity(vecThrow);
			pGrenade->SetLocalAngularVelocity(QAngle(0, 400, 0));
			pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY);
			//pGrenade->m_hOwner = npc;
			//pGrenade->m_pMyWeaponAR2 = this;
			pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());

			// FIXME: arrgg ,this is hard coded into the weapon???
			m_flNextGrenadeCheck = gpGlobals->curtime + 6;// wait six seconds before even looking again to see if a grenade can be thrown.

			m_iClip2--;
		}
		break;


	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Activity
//-----------------------------------------------------------------------------
Activity CWeaponGlauncher::GetPrimaryAttackActivity( void )
{
		return ACT_VM_PRIMARYATTACK;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CWeaponGlauncher::Reload(void)
{
	{
		float fCacheTime = m_flNextSecondaryAttack;


		CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
		if (pPlayer)
		{
			if (m_iClip1 < 1)
			{
				Msg("SDE_R+ \n");
				bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);
				if (fRet)
				{
					WeaponSound(RELOAD);
					m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				}
				return fRet;
			}
			else
			{
				Msg("SDE_R- \n");
				bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD_NOBOLD);
				if (fRet)
				{
					WeaponSound(RELOAD);
					m_flNextSecondaryAttack = GetOwner()->m_flNextAttack = fCacheTime;
				}
				return fRet;
			}
		}
		else
		{
			return false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::AddViewKick( void )
{
	#define	EASY_DAMPEN			0.5f
	#define	MAX_VERTICAL_KICK	1.0f	//Degrees
	#define	SLIDE_LIMIT			2.0f	//Seconds
	
	//Get the view kick
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if ( pPlayer == NULL )
		return;

	DoMachineGunKick( pPlayer, EASY_DAMPEN, MAX_VERTICAL_KICK, m_fFireDuration, SLIDE_LIMIT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponGlauncher::SecondaryAttack( void )
{
	return;
}

#define	COMBINE_MIN_GRENADE_CLEAR_DIST 256

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flDot - 
//			flDist - 
// Output : int
//-----------------------------------------------------------------------------
int CWeaponGlauncher::WeaponRangeAttack2Condition( float flDot, float flDist )
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
	if ( npcOwner->IsMoving())
		return COND_NONE;

	CBaseEntity *pEnemy = npcOwner->GetEnemy();

	if (!pEnemy)
		return COND_NONE;

	Vector vecEnemyLKP = npcOwner->GetEnemyLKP();
	if ( !( pEnemy->GetFlags() & FL_ONGROUND ) && pEnemy->GetWaterLevel() == 0 && vecEnemyLKP.z > (GetAbsOrigin().z + WorldAlignMaxs().z) )
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
	if (random->RandomInt(0,1))
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


	if ( ( vecTarget - npcOwner->GetLocalOrigin() ).Length2D() <= COMBINE_MIN_GRENADE_CLEAR_DIST )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->curtime + 1; // one full second.
		return (COND_NONE);
	}

	// ---------------------------------------------------------------------
	// Are any friendlies near the intended grenade impact area?
	// ---------------------------------------------------------------------
	CBaseEntity *pTarget = NULL;

	while ( ( pTarget = gEntList.FindEntityInSphere( pTarget, vecTarget, COMBINE_MIN_GRENADE_CLEAR_DIST ) ) != NULL )
	{
		//Check to see if the default relationship is hatred, and if so intensify that
		if ( npcOwner->IRelationType( pTarget ) == D_LI )
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

	Vector vecToss = VecCheckThrow( this, npcOwner->GetLocalOrigin() + Vector(0,0,60), vecTarget, 600.0, 0.5 );
	if ( vecToss != vec3_origin )
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
const WeaponProficiencyInfo_t *CWeaponGlauncher::GetProficiencyValues()
{
	static WeaponProficiencyInfo_t proficiencyTable[] =
	{
		{ 7.0,		0.75	},
		{ 5.00,		0.75	},
		{ 10.0/3.0, 0.75	},
		{ 5.0/3.0,	0.75	},
		{ 1.00,		1.0		},
	};

	COMPILE_TIME_ASSERT( ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);

	return proficiencyTable;
}


void CWeaponGlauncher::PrimaryAttack(void)
{
CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
if (!pPlayer)
return;

pPlayer->RumbleEffect(RUMBLE_357, 0, RUMBLE_FLAGS_NONE);

Vector vecSrc = pPlayer->Weapon_ShootPosition();
Vector	vecThrow;
// Don't autoaim on grenade tosses
AngleVectors(pPlayer->EyeAngles() + pPlayer->GetPunchAngle(), &vecThrow);
VectorScale(vecThrow, 1950.0f, vecThrow);

QAngle angles;
VectorAngles(vecThrow, angles);
CGrenadeAR2 *pGrenade = (CGrenadeAR2*)Create("grenade_ar2", vecSrc, angles, pPlayer);
pGrenade->SetModel("models/items/ar3_grenade_noshell.mdl");
pGrenade->SetAbsVelocity(vecThrow);

//pGrenade->SetLocalAngularVelocity( RandomAngle( -400, 400 ) );
pGrenade->SetLocalAngularVelocity(RandomAngle(-5, 5));
pGrenade->SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
pGrenade->SetThrower(GetOwner());
pGrenade->SetDamage(sk_plr_dmg_smg1_grenade.GetFloat());
pPlayer->ViewPunch(QAngle(random->RandomFloat(-12, -8), random->RandomFloat(-10, 10), 0));
//WeaponSound(RELOAD);
CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 1000, 0.2, GetOwner(), SOUNDENT_CHANNEL_WEAPON);

//primary_attack_start
	// If my clip is empty (and I use clips) start reload
	if (UsesClipsForAmmo1() && !m_iClip1)
	{
		Reload();
		return;
	}

	// Only the player fires this way so we can cast


	//pPlayer->DoMuzzleFlash();

	SendWeaponAnim(GetPrimaryAttackActivity());

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

	//pPlayer->FireBullets(info);

	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}

	//Add our view kick in
	AddViewKick();
	//primary_attack_stop

if (m_bFireOnEmpty)
{
	return;
}

SendWeaponAnim(ACT_VM_PRIMARYATTACK);

QAngle	viewPunch;

viewPunch.x = random->RandomFloat(0.3f, 0.3f);
viewPunch.y = random->RandomFloat(0.3f, 0.3f);
viewPunch.z = 0.0f;

pPlayer->ViewPunch(viewPunch);

CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		m_iPrimaryAttacks++;
		gamestats->Event_WeaponFired(pOwner, true, GetClassname());
	}
}