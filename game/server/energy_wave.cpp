//=========== © Copyright 2000 Valve, L.L.C. All rights reserved. =========== 
// 
// The copyright to the contents herein is the property of Valve, L.L.C. 
// The contents may be used and/or copied only with the written permission of 
// Valve, L.L.C., or in accordance with the terms and conditions stipulated in 
// the agreement/contract under which the contents have been supplied. 
// 
// Purpose: The Escort's EWave weapon 
// 
// $Revision: $ 
// $NoKeywords: $ 
//=============================================================================

#include "cbase.h" 
#include "energy_wave.h" 
#include "in_buttons.h" 
#include "gamerules.h" 
//#include

LINK_ENTITY_TO_CLASS(energy_wave, CEnergyWave);

EXTERN_SEND_TABLE(DT_BaseEntity)

IMPLEMENT_SERVERCLASS_ST(CEnergyWave, DT_EWaveEffect)
END_SEND_TABLE()

//--------------------------------------------------------- 
// Save/Restore 
//--------------------------------------------------------- 
BEGIN_DATADESC(CEnergyWave)
END_DATADESC()

//----------------------------------------------------------------------------- 
// Purpose: 
//----------------------------------------------------------------------------- 
void CEnergyWave::Precache(void)
{
	SetClassname("energy_wave");
}

//----------------------------------------------------------------------------- 
// Purpose: 
//----------------------------------------------------------------------------- 
void CEnergyWave::Spawn(void)
{
	Precache();

	// Make it translucent 
	m_nRenderFX = kRenderTransAlpha;
	SetRenderColorA(255);

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_NONE);
	UTIL_SetSize(this, vec3_origin, vec3_origin);

	// Think function 
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//----------------------------------------------------------------------------- 
// Purpose: Create an energy wave 
//-----------------------------------------------------------------------------

CEnergyWave* CEnergyWave::Create(CBaseEntity *pentOwner)
{
	CEnergyWave *pEWave = (CEnergyWave*)CreateEntityByName("energy_wave");

	UTIL_SetOrigin(pEWave, pentOwner->GetLocalOrigin());
	pEWave->SetOwnerEntity(pentOwner);
	pEWave->SetLocalAngles(pentOwner->GetLocalAngles());
	pEWave->Spawn();

	return pEWave;
}

А во второй это :

//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============// 
// 
// Purpose: 
// 
// $NoKeywords: $ 
//=============================================================================//

#ifndef ENERGYWAVE_H 
#define ENERGYWAVE_H 
#ifdef _WIN32 
#pragma once 
#endif

#include "basecombatweapon.h" 
#include "energy_wave.h"

//----------------------------------------------------------------------------- 
// Purpose: Shield 
//----------------------------------------------------------------------------- 
class CEnergyWave : public CBaseEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CEnergyWave, CBaseEntity);
	DECLARE_SERVERCLASS();

public:
	void Spawn(void);
	void Precache(void);

public:
	static CEnergyWave* Create(CBaseEntity *pentOwner);
};

#endif //ENERGYWAVE_H

Всё, а сейчас надо сделать так что б наш хоундей смог сделать энергоудар как в первой части хл = ) Значит так, добовляем опять таки в сервер часть 2 файла: 1)grenade_energy.cpp и 2)grenade_energy.h и в первый вписываем :

//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============ 
// 
// Purpose: Projectile shot by mortar synth. 
// 
// $NoKeywords: $ 
//=============================================================================

#include "cbase.h" 
#include "grenade_energy.h" 
#include "soundent.h" 
#include "player.h" 
#include "hl2_shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

#define ENERGY_GRENADE_LIFETIME 1

ConVar sk_dmg_energy_grenade("sk_dmg_energy_grenade", "0");
ConVar sk_energy_grenade_radius("sk_energy_grenade_radius", "0");

BEGIN_DATADESC(CGrenadeEnergy)

DEFINE_FIELD(m_flMaxFrame, FIELD_INTEGER),
DEFINE_FIELD(m_nEnergySprite, FIELD_INTEGER),
DEFINE_FIELD(m_flLaunchTime, FIELD_TIME),

// Function pointers 
// DEFINE_FUNCTION( Animate ), 
// DEFINE_FUNCTION( GrenadeEnergyTouch ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(grenade_energy, CGrenadeEnergy);

void CGrenadeEnergy::Spawn(void)
{
	Precache();
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLY);

	SetModel("Models/weapons/w_energy_grenade.mdl");

	SetUse(DetonateUse);
	SetTouch(GrenadeEnergyTouch);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_flDamage = sk_dmg_energy_grenade.GetFloat();
	m_DmgRadius = sk_energy_grenade_radius.GetFloat();
	m_takedamage = DAMAGE_YES;
	m_iHealth = 1;

	SetCycle(0.0f);
	m_flModelScale = 0.5;
	m_flLaunchTime = gpGlobals->curtime;

	SetCollisionGroup(HL2COLLISION_GROUP_HOUNDEYE);

	UTIL_SetSize(this, vec3_origin, vec3_origin);

	m_flMaxFrame = (float)modelinfo->GetModelFrameCount(GetModel()) - 1;

}

//------------------------------------------------------------------------------ 
// Purpose : 
// Input : 
// Output : 
//------------------------------------------------------------------------------ 
void CGrenadeEnergy::Shoot(CBaseEntity* pOwner, const Vector &vStart, Vector vVelocity)
{
	CGrenadeEnergy *pEnergy = (CGrenadeEnergy *)CreateEntityByName("grenade_energy");
	pEnergy->Spawn();

	UTIL_SetOrigin(pEnergy, vStart);
	pEnergy->SetAbsVelocity(vVelocity);
	pEnergy->SetOwnerEntity(pOwner);

	pEnergy->SetThink(Animate);
	pEnergy->SetNextThink(gpGlobals->curtime + 0.1f);

	pEnergy->m_nRenderMode = kRenderTransAdd;
	pEnergy->SetRenderColor(160, 160, 160, 255);
	pEnergy->m_nRenderFX = kRenderFxNone;
}

//------------------------------------------------------------------------------ 
// Purpose : 
// Input : 
// Output : 
//------------------------------------------------------------------------------ 
void CGrenadeEnergy::Animate(void)
{
	float flLifeLeft = 1 - (gpGlobals->curtime - m_flLaunchTime) / ENERGY_GRENADE_LIFETIME;

	if (flLifeLeft < 0)
	{
		SetRenderColorA(0);
		SetThink(NULL);
		UTIL_Remove(this);
	}

	SetNextThink(gpGlobals->curtime + 0.01f);

	QAngle angles;
	VectorAngles(GetAbsVelocity(), angles);
	SetLocalAngles(angles);

	SetNextThink(gpGlobals->curtime + 0.1f);

	StudioFrameAdvance();

	SetRenderColorA(flLifeLeft);
}

void CGrenadeEnergy::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

void CGrenadeEnergy::GrenadeEnergyTouch(CBaseEntity *pOther)
{
	if (pOther->m_takedamage)
	{
		float flLifeLeft = 1 - (gpGlobals->curtime - m_flLaunchTime) / ENERGY_GRENADE_LIFETIME;

		if (pOther->GetFlags() & (FL_CLIENT))
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pOther;
			float flKick = 120 * flLifeLeft;
			pPlayer->m_Local.m_vecPunchAngle.SetX(flKick * (random->RandomInt(0, 1) == 1) ? -1 : 1);
			pPlayer->m_Local.m_vecPunchAngle.SetY(flKick * (random->RandomInt(0, 1) == 1) ? -1 : 1);
		}
		float flDamage = m_flDamage * flLifeLeft;
		if (flDamage < 1)
		{
			flDamage = 1;
		}

		trace_t tr;
		tr = GetTouchTrace();
		CTakeDamageInfo info(this, GetThrower(), m_flDamage * flLifeLeft, DMG_SONIC);
		CalculateMeleeDamageForce(&info, (tr.endpos - tr.startpos), tr.endpos);
		pOther->TakeDamage(info);
	}
	Detonate();
}

void CGrenadeEnergy::Detonate(void)
{
	m_takedamage = DAMAGE_NO;
	UTIL_Remove(this);
}

void CGrenadeEnergy::Precache(void)
{
	engine->PrecacheModel("Models/weapons/w_energy_grenade.mdl");
}