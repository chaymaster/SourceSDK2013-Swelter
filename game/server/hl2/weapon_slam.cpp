//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Отредактено Lolmen'om aka Протасов Виталий Cwall lan :D
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "grenade_tripmine.h"
#include "grenade_satchel.h"
#include "entitylist.h"
#include "weapon_slam.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "baseentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define	SLAM_PRIMARY_VOLUME		450

BEGIN_DATADESC(CWeapon_SLAM)

DEFINE_FIELD(m_tSlamState, FIELD_INTEGER),
DEFINE_FIELD(m_bDetonatorArmed, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedDetonatorDraw, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedDetonatorHolster, FIELD_BOOLEAN),
DEFINE_FIELD(m_bNeedReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_bClearReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_bThrowSatchel, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAttachSatchel, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAttachTripmine, FIELD_BOOLEAN),
DEFINE_FIELD(m_flWallSwitchTime, FIELD_TIME),

// Указатели функций
DEFINE_FUNCTION(SlamTouch),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST(CWeapon_SLAM, DT_Weapon_SLAM)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_slam, CWeapon_SLAM);
PRECACHE_WEAPON_REGISTER(weapon_slam);

acttable_t	CWeapon_SLAM::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeapon_SLAM);


void CWeapon_SLAM::Spawn()
{
	BaseClass::Spawn();

	Precache();

	UTIL_SetSize(this, Vector(-4, -4, -2), Vector(4, 4, 2));

	FallInit();// подготовится к падеию, мот из за этого исчезает?

	SetThink(NULL);

	m_tSlamState = SLAM_TRIPMINE_READY;
	m_flWallSwitchTime = 0;

	// Прибавить 1 кусок oт обычного ammo когда подняли первый раз
	m_iClip2 = 1;
}

void CWeapon_SLAM::Precache(void)
{
	BaseClass::Precache();

	UTIL_PrecacheOther("npc_tripmine");
	UTIL_PrecacheOther("npc_satchel");

	PrecacheScriptSound("Weapon_SLAM.ThrowMode");
	PrecacheScriptSound("Weapon_SLAM.TripMineMode");
	PrecacheScriptSound("Weapon_SLAM.SatchelDetonate");
	PrecacheScriptSound("Weapon_SLAM.TripMineAttach");
	PrecacheScriptSound("Weapon_SLAM.SatchelThrow");
	PrecacheScriptSound("Weapon_SLAM.SatchelAttach");

}

//------------------------------------------------------------------------------
// Purpose : Переопределение для использования slam's поднятия прикасновения
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeapon_SLAM::SetPickupTouch(void)
{
	SetTouch(&CWeapon_SLAM::SlamTouch);
}
//-----------------------------------------------------------------------------
// Purpose: Переопределение ну даём корректный ammo
// Input  : pOther -  entity которая дотронулась до меня
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SlamTouch(CBaseEntity *pOther)
{
	// Can I even pick stuff up?
	//if (pOther->IsEFlagSet(EFL_NO_WEAPON_PICKUP))
	//	return;

	// ---------------------------------------------------
	//  First give weapon to touching entity if allowed
	// ---------------------------------------------------
	BaseClass::DefaultTouch(pOther);
}

//------------------------------------------------------------------------------
// Purpose : Slam в Кабуре
// Input   :
// Output  :
//------------------------------------------------------------------------------
bool CWeapon_SLAM::Holster(CBaseCombatWeapon *pSwitchingTo)
{
	SetThink(NULL);
	return BaseClass::Holster(pSwitchingTo);
}

//-----------------------------------------------------------------------------
// Purpose: SLAM не перезаряжается, но далжно позвать холостую чтобы обновить статус
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_SLAM::Reload(void)
{
	WeaponIdle();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Первичная отака
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::PrimaryAttack(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return;
	}

	if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		return;
	}

	switch (m_tSlamState)
	{
	case SLAM_TRIPMINE_READY:
		if (CanAttachSLAM())
		{
			StartTripmineAttach();
		}
		break;
	case SLAM_SATCHEL_THROW:
		StartSatchelThrow();
		break;
	case SLAM_SATCHEL_ATTACH:
		StartSatchelAttach();
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Вторичная атака переключается между satchel(сумкой)и tripmine(противопехотной миной) 
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SecondaryAttack(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return;
	}

	if (m_bDetonatorArmed)
	{
		StartSatchelDetonate();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Сумка детонирует
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SatchelDetonate()
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "npc_satchel")) != NULL)
	{
		CSatchelCharge *pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() && GetOwner() && pSatchel->GetThrower() == GetOwner())
		{
			pSatchel->Use(GetOwner(), GetOwner(), USE_ON, 0);
		}
	}
	// Звук для нажатого детонатора
	EmitSound("Weapon_SLAM.SatchelDetonate");

	m_bDetonatorArmed = false;
}

//-----------------------------------------------------------------------------
// Purpose: Возвращает true если там в мире есть незаряжённые разряды принадлежащие игроку
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_SLAM::AnyUndetonatedCharges(void)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = gEntList.FindEntityByClassname(pEntity, "npc_satchel")) != NULL)
	{
		CSatchelCharge* pSatchel = dynamic_cast<CSatchelCharge *>(pEntity);
		if (pSatchel->m_bIsLive && pSatchel->GetThrower() && pSatchel->GetThrower() == GetOwner())
		{
			return true;
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Начинаем детонацию сумки
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::StartSatchelDetonate()
{

	if (GetActivity() != ACT_SLAM_DETONATOR_IDLE && GetActivity() != ACT_SLAM_THROW_IDLE)
		return;
	// -----------------------------------------
	//  Играем Детонационную анимацию
	// -----------------------------------------
	if (m_bNeedReload)
	{
		SendWeaponAnim(ACT_SLAM_DETONATOR_DETONATE);
	}
	else if (m_tSlamState == SLAM_SATCHEL_ATTACH)
	{
		SendWeaponAnim(ACT_SLAM_STICKWALL_DETONATE);
	}
	else if (m_tSlamState == SLAM_SATCHEL_THROW)
	{
		SendWeaponAnim(ACT_SLAM_THROW_DETONATE);
	}
	else
	{
		return;
	}
	SatchelDetonate();

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose: Противопехотка присоеденение
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::TripmineAttach(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return;
	}

	m_bAttachTripmine = false;

	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->EyeDirection3D();

	trace_t tr;

	UTIL_TraceLine(vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			QAngle angles;
			VectorAngles(tr.plane.normal, angles);
			angles.x += 90;

			CBaseEntity *pEnt = CBaseEntity::Create("npc_tripmine", tr.endpos + tr.plane.normal * 3, angles, NULL);

			CTripmineGrenade *pMine = (CTripmineGrenade *)pEnt;
			pMine->m_hOwner = GetOwner();

			pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);

			EmitSound("Weapon_SLAM.TripMineAttach");
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Начинаем прикрепление Противопехотки
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::StartTripmineAttach(void)
{
	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());
	if (!pPlayer)
	{
		return;
	}

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pPlayer->EyePosition();

	QAngle angles = pPlayer->GetLocalAngles();

	AngleVectors(angles, &vecAiming);

	trace_t tr;

	UTIL_TraceLine(vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		// ALERT( at_console, "hit %f\n", tr.flFraction );

		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			// player "shoot" animation
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			// -----------------------------------------
			//  Play attach animation
			// -----------------------------------------

			if (m_bDetonatorArmed)
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ATTACH);
			}
			else
			{
				SendWeaponAnim(ACT_SLAM_TRIPMINE_ATTACH);
			}

			m_bNeedReload = true;
			m_bAttachTripmine = true;
			m_bNeedDetonatorDraw = m_bDetonatorArmed;
		}
		else
		{
			// ALERT( at_console, "no deploy\n" );
		}
	}

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
	//	SetWeaponIdleTime( gpGlobals->curtime + SequenceDuration() );
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SatchelThrow(void)
{

	m_bThrowSatchel = false;

	// Тока выстрелы игрока бросаем
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	Vector vecSrc = pPlayer->WorldSpaceCenter();
	Vector vecFacing = pPlayer->BodyDirection3D();
	vecSrc = vecSrc + vecFacing * 18.0;
	// BUGBUG: это потому что vecSrc не для Weapon_ShootPosition()???
	vecSrc.z += 24.0f;

	Vector vecThrow;
	GetOwner()->GetVelocity(&vecThrow, NULL);
	vecThrow += vecFacing * 500;

	// Игрок могет повернутся лицом к стене в течении проигрывания throw анимки
	// Мы не хочем чтобы противопехотка SLAM вошла в стену
	if (CanAttachSLAM())
	{
		vecThrow = vecFacing;
		vecSrc = pPlayer->WorldSpaceCenter() + vecFacing * 5.0;
	}

	CSatchelCharge *pSatchel = (CSatchelCharge*)Create("npc_satchel", vecSrc, vec3_angle, GetOwner());
	pSatchel->SetThrower(GetOwner());
	pSatchel->ApplyAbsVelocityImpulse(vecThrow);
	pSatchel->SetLocalAngularVelocity(QAngle(0, 400, 0));
	pSatchel->m_bIsLive = true;
	pSatchel->m_pMyWeaponSLAM = this;

	pPlayer->RemoveAmmo(1, m_iSecondaryAmmoType);

	// Играем противопехоткин звук
	EmitSound("Weapon_SLAM.SatchelThrow");
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::StartSatchelThrow(void)
{
	// -----------------------------------------
	//  Играем анимку Взвода
	// -----------------------------------------
	if (m_bDetonatorArmed)
	{
		SendWeaponAnim(ACT_SLAM_THROW_THROW);
	}
	else
	{
		SendWeaponAnim(ACT_SLAM_THROW_THROW_ND);
		if (!m_bDetonatorArmed)
		{
			m_bDetonatorArmed = true;
			m_bNeedDetonatorDraw = true;
		}
	}

	m_bNeedReload = true;
	m_bThrowSatchel = true;

	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
	m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SatchelAttach(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return;
	}

	m_bAttachSatchel = false;

	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->BodyDirection2D();

	trace_t tr;

	UTIL_TraceLine(vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			QAngle angles;
			VectorAngles(tr.plane.normal, angles);
			angles.y -= 90;
			angles.z -= 90;
			tr.endpos.z -= 6.0f;

			EmitSound("Weapon_SLAM.SatchelAttach");

			CSatchelCharge *pSatchel = (CSatchelCharge*)CBaseEntity::Create("npc_satchel", tr.endpos + tr.plane.normal * 3, angles, NULL);
			pSatchel->SetMoveType(MOVETYPE_FLY); // no gravity
			pSatchel->m_bIsAttached = true;
			pSatchel->m_bIsLive = true;
			pSatchel->SetThrower(GetOwner());
			pSatchel->SetOwnerEntity(((CBaseEntity*)GetOwner()));
			pSatchel->m_pMyWeaponSLAM = this;

			pOwner->RemoveAmmo(1, m_iSecondaryAmmoType);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::StartSatchelAttach(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return;
	}

	Vector vecSrc = pOwner->Weapon_ShootPosition();
	Vector vecAiming = pOwner->BodyDirection2D();

	trace_t tr;

	UTIL_TraceLine(vecSrc, vecSrc + (vecAiming * 128), MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		CBaseEntity *pEntity = tr.m_pEnt;
		if (pEntity && !(pEntity->GetFlags() & FL_CONVEYOR))
		{
			// Only the player fires this way so we can cast
			CBasePlayer *pPlayer = ToBasePlayer(pOwner);

			// player "shoot" animation
			pPlayer->SetAnimation(PLAYER_ATTACK1);

			// -----------------------------------------
			//  Play attach animation
			// -----------------------------------------
			if (m_bDetonatorArmed)
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ATTACH);
			}
			else
			{
				SendWeaponAnim(ACT_SLAM_STICKWALL_ND_ATTACH);
				if (!m_bDetonatorArmed)
				{
					m_bDetonatorArmed = true;
					m_bNeedDetonatorDraw = true;
				}
			}

			m_bNeedReload = true;
			m_bAttachSatchel = true;

			m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SetSlamState(SlamState_t newState)
{
	// Set set and set idle time so animation gets updated with state change
	m_tSlamState = newState;
	SetWeaponIdleTime(gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::SLAMThink(void)
{
	if (m_flWallSwitchTime > gpGlobals->curtime)
		return;


	// If not in tripmine mode we need to check to see if we are close to
	// a wall. If we are we go into satchel_attach mode
	CBaseCombatCharacter *pOwner = GetOwner();

	if ((pOwner && pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0))
	{
		if (CanAttachSLAM())
		{
			if (m_tSlamState == SLAM_SATCHEL_THROW)
			{
				SetSlamState(SLAM_TRIPMINE_READY);
				int iAnim = m_bDetonatorArmed ? ACT_SLAM_THROW_TO_STICKWALL : ACT_SLAM_THROW_TO_TRIPMINE_ND;
				SendWeaponAnim(iAnim);
				m_flWallSwitchTime = gpGlobals->curtime + SequenceDuration();
				m_bNeedReload = false;
			}
		}
		else
		{
			if (m_tSlamState == SLAM_TRIPMINE_READY)
			{
				SetSlamState(SLAM_SATCHEL_THROW);
				int iAnim = m_bDetonatorArmed ? ACT_SLAM_STICKWALL_TO_THROW : ACT_SLAM_TRIPMINE_TO_THROW_ND;
				SendWeaponAnim(iAnim);
				m_flWallSwitchTime = gpGlobals->curtime + SequenceDuration();
				m_bNeedReload = false;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeapon_SLAM::CanAttachSLAM(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	Vector vecSrc, vecAiming;

	// Take the eye position and direction
	vecSrc = pOwner->EyePosition();

	QAngle angles = pOwner->GetLocalAngles();

	AngleVectors(angles, &vecAiming);

	trace_t tr;

	Vector	vecEnd = vecSrc + (vecAiming * 42);
	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction < 1.0)
	{
		// Don't attach to a living creature
		if (tr.m_pEnt)
		{
			CBaseEntity *pEntity = tr.m_pEnt;
			CBaseCombatCharacter *pBCC = ToBaseCombatCharacter(pEntity);
			if (pBCC)
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Переопределение что SLAM хочет стрелять вторичкой но нету патронов
//			но сумка в мире
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
	{
		return;
	}

	SLAMThink();

	if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		SecondaryAttack();
	}
	else if (!m_bNeedReload && (pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= gpGlobals->curtime))
	{
		PrimaryAttack();
	}

	// -----------------------
	//  Не нажимать кнопки
	// -----------------------
	else
	{
		WeaponIdle();
		return;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Переключится к следующему лутшему оружию
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::WeaponSwitch(void)
{
	// Примечение мы можем взять SLAM опять, когда переключаем
	// оружия, когда у нас детонатор в руках сохранился на взводе
	//  SLAMs могут быть дергаными, но не надо
	// и развёртывающая функция найдёт невзведённые детонаторы
	// и мы взведём
	bool saveState = m_bDetonatorArmed;
	CBaseCombatCharacter *pOwner = GetOwner();
	pOwner->SwitchToNextBestWeapon(pOwner->GetActiveWeapon());
	if (pOwner->GetActiveWeapon() == this)
	{
		m_bDetonatorArmed = saveState;
	}

	// Не взведено и патронов нету
	if (!m_bDetonatorArmed && pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
	{
		pOwner->ClearActiveWeapon();
	}

}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeapon_SLAM::WeaponIdle(void)
{
	// Готов к переключению анимок?
	if (HasWeaponIdleTimeElapsed())
	{
		// не разрешать минке прикрепляться пока не пройдёт лодырьство idle
		m_flWallSwitchTime = gpGlobals->curtime + 50;

		if (m_bClearReload)
		{
			m_bNeedReload = false;
			m_bClearReload = false;
		}
		CBaseCombatCharacter *pOwner = GetOwner();
		if (!pOwner)
		{
			return;
		}

		int iAnim = 0;

		if (m_bThrowSatchel)
		{
			SatchelThrow();
			if (m_bDetonatorArmed && !m_bNeedDetonatorDraw)
			{
				iAnim = ACT_SLAM_THROW_THROW2;
			}
			else
			{
				iAnim = ACT_SLAM_THROW_THROW_ND2;
			}
		}
		else if (m_bAttachSatchel)
		{
			SatchelAttach();
			if (m_bDetonatorArmed && !m_bNeedDetonatorDraw)
			{
				iAnim = ACT_SLAM_STICKWALL_ATTACH2;
			}
			else
			{
				iAnim = ACT_SLAM_STICKWALL_ND_ATTACH2;
			}
		}
		else if (m_bAttachTripmine)
		{
			TripmineAttach();
			iAnim = ACT_SLAM_TRIPMINE_ATTACH2;
		}
		else if (m_bNeedReload)
		{
			// Если другой имеет патроны отображать SLAM тип
			if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) > 0)
			{
				switch (m_tSlamState)
				{
				case SLAM_TRIPMINE_READY:
				{
											////iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_STICKWALL_DRAW : ACT_SLAM_TRIPMINE_DRAW;
				}
					break;
				case SLAM_SATCHEL_ATTACH:
				{
											if (m_bNeedDetonatorHolster)
											{
												iAnim = ACT_SLAM_STICKWALL_DETONATOR_HOLSTER;
												m_bNeedDetonatorHolster = false;
											}
											else if (m_bDetonatorArmed)
											{
												iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_STICKWALL_DRAW : ACT_SLAM_STICKWALL_DRAW;
												m_bNeedDetonatorDraw = false;
											}
											else
											{
												iAnim = ACT_SLAM_STICKWALL_ND_DRAW;
											}
				}
					break;
				case SLAM_SATCHEL_THROW:
				{
										   if (m_bNeedDetonatorHolster)
										   {
											   iAnim = ACT_SLAM_THROW_DETONATOR_HOLSTER;
											   m_bNeedDetonatorHolster = false;
										   }
										   else if (m_bDetonatorArmed)
										   {
											   iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_THROW_DRAW : ACT_SLAM_THROW_DRAW;
											   m_bNeedDetonatorDraw = false;
										   }
										   else
										   {
											   iAnim = ACT_SLAM_THROW_ND_DRAW;
										   }
				}
					break;
				}
				m_bClearReload = true;
			}
			// еси патроны не на взводе, застой просто детонатор
			else if (m_bDetonatorArmed)
			{
				iAnim = m_bNeedDetonatorDraw ? ACT_SLAM_DETONATOR_DRAW : ACT_SLAM_DETONATOR_IDLE;
				m_bNeedDetonatorDraw = false;
			}
			else
			{
				pOwner->Weapon_Drop(this);
				UTIL_Remove(this);
			}
		}
		else if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
		{
			pOwner->Weapon_Drop(this);
			UTIL_Remove(this);
		}

		// еси нам не надо перезярядка просто начать нужный idle
		else
		{
			switch (m_tSlamState)
			{
			case SLAM_TRIPMINE_READY:
			{
										iAnim = m_bDetonatorArmed ? ACT_SLAM_STICKWALL_IDLE : ACT_SLAM_TRIPMINE_IDLE;
										m_flWallSwitchTime = 0;
			}
				break;
			case SLAM_SATCHEL_THROW:
			{
									   if (m_bNeedDetonatorHolster)
									   {
										   iAnim = ACT_SLAM_THROW_DETONATOR_HOLSTER;
										   m_bNeedDetonatorHolster = false;
									   }
									   else
									   {
										   iAnim = m_bDetonatorArmed ? ACT_SLAM_THROW_IDLE : ACT_SLAM_THROW_ND_IDLE;
										   m_flWallSwitchTime = 0;
									   }
			}
				break;
			case SLAM_SATCHEL_ATTACH:
			{
										if (m_bNeedDetonatorHolster)
										{
											iAnim = ACT_SLAM_STICKWALL_DETONATOR_HOLSTER;
											m_bNeedDetonatorHolster = false;
										}
										else
										{
											iAnim = m_bDetonatorArmed ? ACT_SLAM_STICKWALL_IDLE : ACT_SLAM_STICKWALL_ND_IDLE;
											m_flWallSwitchTime = 0;
										}
			}
				break;
			}
		}
		SendWeaponAnim(iAnim);
	}
}

bool CWeapon_SLAM::Deploy(void)
{
	CBaseCombatCharacter *pOwner = GetOwner();
	if (!pOwner)
	{
		return false;
	}

	m_bDetonatorArmed = AnyUndetonatedCharges();


	SetThink(SLAMThink);
	SetNextThink(gpGlobals->curtime + 0.1f);

	SetModel(GetViewModel());

	// ------------------------------
	//  Брать правельную отображение анимки
	// ------------------------------
	int iActivity;

	// Если детонатор уже взведён
	m_bNeedReload = false;
	if (m_bDetonatorArmed)
	{
		if (pOwner->GetAmmoCount(m_iSecondaryAmmoType) <= 0)
		{
			iActivity = ACT_SLAM_DETONATOR_DRAW;
			m_bNeedReload = true;
		}
		else if (CanAttachSLAM())
		{
			iActivity = ACT_SLAM_DETONATOR_STICKWALL_DRAW;
			SetSlamState(SLAM_TRIPMINE_READY);
		}
		else
		{
			iActivity = ACT_SLAM_DETONATOR_THROW_DRAW;
			SetSlamState(SLAM_SATCHEL_THROW);
		}
	}
	else
	{
		if (CanAttachSLAM())
		{
			iActivity = ACT_SLAM_TRIPMINE_DRAW;
			SetSlamState(SLAM_TRIPMINE_READY);
		}
		else
		{
			iActivity = ACT_SLAM_THROW_ND_DRAW;
			SetSlamState(SLAM_SATCHEL_THROW);
		}
	}

	return DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), iActivity, (char*)GetAnimPrefix());
}

//-----------------------------------------------------------------------------
// Purpose: Конструктер
// Input  :
// Output :
//-----------------------------------------------------------------------------
CWeapon_SLAM::CWeapon_SLAM(void)
{
	m_tSlamState = SLAM_SATCHEL_THROW;
	m_bDetonatorArmed = false;
	m_bNeedReload = true;
	m_bClearReload = false;
	m_bThrowSatchel = false;
	m_bAttachSatchel = false;
	m_bAttachTripmine = false;
	m_bNeedDetonatorDraw = false;
	m_bNeedDetonatorHolster = false;
}
//const WeaponProficiencyInfo_t *CWeapon_SLAM::GetProficiencyValues()
//{
//	static WeaponProficiencyInfo_t proficiencyTable[] =
//	{
//		{ 7.0, 0.75 },
//		{ 5.00, 0.75 },
//		{ 3.0, 0.85 },
//		{ 5.0 / 3.0, 0.75 },
//		{ 1.00, 1.0 },
//	};
//
//	COMPILE_TIME_ASSERT(ARRAYSIZE(proficiencyTable) == WEAPON_PROFICIENCY_PERFECT + 1);
//
//	return proficiencyTable;
//}
