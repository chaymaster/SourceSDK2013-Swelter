//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		358 - hand gun
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
// CWeapon358
//-----------------------------------------------------------------------------

class CWeapon358 : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CWeapon358, CBaseHLCombatWeapon);
public:

	CWeapon358(void);

	void	PrimaryAttack(void);
	bool	Reload(void);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	float	WeaponAutoAimScale()	{ return 0.6f; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(weapon_358, CWeapon358);

PRECACHE_WEAPON_REGISTER(weapon_358);

IMPLEMENT_SERVERCLASS_ST(CWeapon358, DT_Weapon358)
END_SEND_TABLE()

BEGIN_DATADESC(CWeapon358)
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeapon358::CWeapon358(void)
{
	m_bReloadsSingly = false;
	m_bFiresUnderwater = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon358::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	switch (pEvent->event)
	{
	case EVENT_WEAPON_RELOAD:
	{
								CEffectData data;

								// Emit six spent shells
								for (int i = 0; i < 6; i++)
								{
									data.m_vOrigin = pOwner->WorldSpaceCenter() + RandomVector(-4, 4);
									data.m_vAngles = QAngle(90, random->RandomInt(0, 360), 0);
									data.m_nEntIndex = entindex();

									DispatchEffect("ShellEject", data);
								}

								break;
	}
	}
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CWeapon358::PrimaryAttack(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (!pPlayer)
	{
		return;
	}

	if (m_iClip1 <= 0)
	{
		if (!m_bFireOnEmpty)
		{
			Reload();
		}
		else
		{
			WeaponSound(EMPTY);
			m_flNextPrimaryAttack = 0.15;
		}

		return;
	}

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pPlayer, true, GetClassname());

	pPlayer->DoMuzzleFlash();

	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = gpGlobals->curtime + 0.6f;
	m_flNextSecondaryAttack = gpGlobals->curtime + 0.6f;

	m_iClip1--;

	Vector vecSrc = pPlayer->Weapon_ShootPosition();
	Vector vecAiming = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT);

	pPlayer->FireBullets(1, vecSrc, vecAiming, vec3_origin, MAX_TRACE_LENGTH, m_iPrimaryAmmoType, 0);

	pPlayer->SetMuzzleFlashTime(gpGlobals->curtime + 0.5);

	//Disorient the player
	QAngle angles = pPlayer->GetLocalAngles();

	angles.x += random->RandomInt(-1, 1);
	angles.y += random->RandomInt(-1, 1);
	angles.z = 0;

	pPlayer->SnapEyeAngles(angles);

	pPlayer->ViewPunch(QAngle(-8, random->RandomFloat(-1, 1), 0));
	if (m_iClip1 >= 1)
	{
		//CSoundEnt::InsertSound(SOUND_COMBAT, GetAbsOrigin(), 600, 0.2, GetOwner());
		WeaponSound(SINGLE);
	}
	else
	{
		WeaponSound(EMPTY_SHOT);
	}
	if (!m_iClip1 && pPlayer->GetAmmoCount(m_iPrimaryAmmoType) <= 0)
	{
		// HEV suit - indicate out of ammo condition
		pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
	}
}

bool CWeapon358::Reload(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (m_iClip1 < 1 && pPlayer)
	{
		bool fRet = DefaultReload(GetMaxClip1(), GetMaxClip2(), ACT_VM_RELOAD);

		if (fRet)
		{
			WeaponSound(RELOAD);
		}
		return fRet;
	}
	else
	{
		return false;
	}
}