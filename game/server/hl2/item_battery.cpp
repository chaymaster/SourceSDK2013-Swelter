//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Handling for the suit batteries.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hl2_player.h"
#include "basecombatweapon.h"
#include "gamerules.h"
#include "items.h"
#include "engine/IEngineSound.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CItemBattery : public CItem
{
public:
	DECLARE_CLASS( CItemBattery, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/battery.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel( "models/items/battery.mdl");
		PrecacheModel( "models/props_se/citizen_tech/battery.mdl");

		PrecacheScriptSound( "ItemBattery.Touch" );

	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		CHL2_Player *pHL2Player = dynamic_cast<CHL2_Player *>( pPlayer );

		//Msg("BATTERY_PICKUP_1 \n");

		//Vector vecForward;
		//AngleVectors(pPlayer->EyeAngles(), &vecForward);
		//CBaseEntity *pEjectProp = (CBaseEntity *)CreateEntityByName("prop_physics_override");
		//
		//if (pEjectProp)
		//{
		//	Vector vecOrigin = pPlayer->GetAbsOrigin() + vecForward * 32 + Vector(0, 0, 35);
		//	QAngle vecAngles(0, pPlayer->GetAbsAngles().y - 10, 0);
		//	pEjectProp->SetAbsOrigin(vecOrigin);
		//	pEjectProp->SetAbsAngles(vecAngles);
		//	pEjectProp->KeyValue("model", "models/props_se/citizen_tech/battery.mdl");
		//	pEjectProp->KeyValue("solid", "1");
		//	pEjectProp->KeyValue("targetname", "EjectProp");
		//	pEjectProp->KeyValue("spawnflags", "260");
		//	DispatchSpawn(pEjectProp);
		//	pEjectProp->Activate();
		//	pEjectProp->Teleport(&vecOrigin, &vecAngles, NULL);
		//}

		return ( pHL2Player && pHL2Player->ApplyBattery() );
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);
PRECACHE_REGISTER(item_battery);

