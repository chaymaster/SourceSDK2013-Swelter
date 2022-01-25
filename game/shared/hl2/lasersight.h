#ifndef LASERSIGHT_H
#define LASERSIGHT_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "weapon_rpg.h"

#ifdef CLIENT_DLL
#include "c_hl2mp_player.h"
#include "model_types.h"
#include "beamdraw.h"
#include "fx_line.h"
#include "view.h"
#else
#include "basecombatcharacter.h"
#include "movie_explosion.h"
#include "soundent.h"
#include "player.h"
#include "rope.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "explode.h"
#include "util.h"
#include "in_buttons.h"
#include "shake.h"
#include "te_effect_dispatch.h"
#include "triggers.h"
#include "smoke_trail.h"
#include "collisionutils.h"
#include "hl2_shareddefs.h"
#include "basehlcombatweapon.h"
#endif

#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Laser dot control
//-----------------------------------------------------------------------------
CBaseEntity *CreateLaserSight(const Vector &origin, CBaseEntity *pOwner, bool bVisibleDot);
void SetLaserSightTarget(CBaseEntity *pLaserDot, CBaseEntity *pTarget);
void EnableLaserSight(CBaseEntity *pLaserDot, bool bEnable);

#ifdef CLIENT_DLL
#define CLaserSight C_LaserSight
#endif


//-----------------------------------------------------------------------------
// Laser Dot ç
//-----------------------------------------------------------------------------
class CLaserSight : public CBaseEntity
{

	DECLARE_CLASS(CLaserSight, CBaseEntity);
public:

	CLaserSight(void);
	~CLaserSight(void);

	static CLaserSight *Create(const Vector &origin, CBaseEntity *pOwner = NULL, bool bVisibleDot = true);

	void    SetTargetEntity(CBaseEntity *pTarget) { m_hTargetEnt = pTarget; }
	CBaseEntity *GetTargetEntity(void) { return m_hTargetEnt; }

	void    SetLaserPosition(const Vector &origin, const Vector &normal);
	Vector  GetChasePosition();
	void    TurnOn(void);
	void    TurnOff(void);
	bool    IsOn() const { return m_bIsOn; }

	void    Toggle(void);

	int             ObjectCaps() { return (BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }

	void    MakeInvisible(void);

#ifdef CLIENT_DLL

	virtual bool                    IsTransparent(void) { return true; }
	virtual RenderGroup_t   GetRenderGroup(void) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int                             DrawModel(int flags);
	virtual void                    OnDataChanged(DataUpdateType_t updateType);
	virtual bool                    ShouldDraw(void) { return (IsEffectActive(EF_NODRAW) == false); }

	CMaterialReference      m_hSpriteMaterial;
#endif

protected:
	Vector                          m_vecSurfaceNormal;
	EHANDLE                         m_hTargetEnt;
	bool                            m_bVisibleLaserDot;
	bool                            m_bIsOn;

	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();
public:
	CLaserSight                     *m_pNext;
};

class ILaserSight
{
private:
	CBaseHLCombatWeapon * _weap;
	CBasePlayer * _c;

public:
	virtual void    CreateLaserPointer();
	virtual void    UpdateLaserPosition(Vector vecMuzzlePos = vec3_origin, Vector vecEndPos = vec3_origin);
	virtual Vector  GetLaserPosition();

	~ILaserSight();
	ILaserSight(CBaseHLCombatWeapon * weap);

#ifndef CLIENT_DLL
	CHandle<CLaserSight>    m_hLaserDot;
#endif
};


#define RPG_BEAM_SPRITE         "effects/laser1.vmt"
#define RPG_BEAM_SPRITE_NOZ     "effects/laser1_noz.vmt"
#define RPG_LASER_SPRITE        "sprites/redglow1"
#endif