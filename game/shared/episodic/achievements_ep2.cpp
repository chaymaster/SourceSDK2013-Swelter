//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================


#include "cbase.h"

#ifdef GAME_DLL

#include "achievementmgr.h"
#include "baseachievement.h"

// Ep2-specific macro that sets game dir filter.  We need this because Ep1/Ep2/... share a binary so we need runtime check against running game.
#define DECLARE_EP2_MAP_EVENT_ACHIEVEMENT( achievementID, achievementName, iPointValue )					\
	DECLARE_MAP_EVENT_ACHIEVEMENT_(achievementID, achievementName, "Swelter", iPointValue, false)

#define DECLARE_EP2_MAP_EVENT_ACHIEVEMENT_HIDDEN( achievementID, achievementName, iPointValue )					\
	DECLARE_MAP_EVENT_ACHIEVEMENT_(achievementID, achievementName, "Swelter", iPointValue, true)



class CAchievementEp2FindAllRadarCaches : public CBaseAchievement
{
	virtual void Init()
	{
		static const char *szComponents[] =
		{
			"ACH_SWELTER_RADCACHE_POLICE", "ACH_SWELTER_RADCACHE_WAREHOUSE_1", "ACH_SWELTER_RADCACHE_HANGAR", "ACH_SWELTER_RADCACHE_TRUCK", "ACH_SWELTER_RADCACHE_CHECKPOINT", "ACH_SWELTER_RADCACHE_BRIDGE", "AACH_SWELTER_RADCACHE_HLEV", "ACH_SWELTER_RADCACHE_POLE", "ACH_SWELTER_RADCACHE_357", "ACH_SWELTER_RADCACHE_ANARHY"
		};
		SetFlags(ACH_HAS_COMPONENTS | ACH_LISTEN_COMPONENT_EVENTS | ACH_SAVE_GLOBAL);
		m_pszComponentNames = szComponents;
		m_iNumComponents = ARRAYSIZE(szComponents);
		SetComponentPrefix("ACH_SWELTER_RADCACHE_");
		SetGameDirFilter("Swelter");
		SetGoal(m_iNumComponents);
	}
};
DECLARE_ACHIEVEMENT(CAchievementEp2FindAllRadarCaches, ACHIEVEMENT_ACH_SWELTER_RADAR, "ACH_SWELTER_RADAR", 10);



// achievements which are won by a map event firing once
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_END_ANARCHY, "ACH_SWELTER_END_ANARCHY", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_END_RESIST, "ACH_SWELTER_END_RESIST", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_VORT, "ACH_SWELTER_VORT", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_AK, "ACH_SWELTER_AK", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_SPAWN, "ACH_SWELTER_SPAWN", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_TURRET, "ACH_SWELTER_TURRET", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_HELI, "ACH_SWELTER_HELI", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_APERTURE, "ACH_SWELTER_APERTURE", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT_HIDDEN(ACHIEVEMENT_ACH_SWELTER_TEA, "ACH_SWELTER_TEA", 10);
DECLARE_EP2_MAP_EVENT_ACHIEVEMENT(ACHIEVEMENT_ACH_SWELTER_TRAIN, "ACH_SWELTER_TRAIN", 10);

#endif // GAME_DLL