//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:		Projectile shot from the AR2 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPONAR2_H
#define	WEAPONAR2_H

#include "basegrenade_shared.h"
#include "basehlcombatweapon.h"

class CWeaponAR2 : public CHLMachineGun
{
public:
	DECLARE_CLASS(CWeaponAR2, CHLMachineGun);

	CWeaponAR2();

	DECLARE_SERVERCLASS();

	void	ItemPostFrame(void);
	void	Precache(void);
	bool	Deploy(void);

	void	PrimaryAttack(void);
	void	SecondaryAttack(void);
	void	HoldIronsight(void);
	void	DelayedAttack(void);
	void	SecondaryEject(void); //new
	void	SecondaryEjectSpawn(void); //new

	const char *GetTracerType(void) { return "AR2Tracer"; }

	void	AddViewKick(void);

	void	FireNPCPrimaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	FireNPCSecondaryAttack(CBaseCombatCharacter *pOperator, bool bUseWeaponAngles);
	void	Operator_ForceNPCFire(CBaseCombatCharacter  *pOperator, bool bSecondary);
	void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	int		GetMinBurst(void) { return 4; }
	int		GetMaxBurst(void) { return 5; }
	float	GetFireRate(void) { return 0.1f; } //{ return 0.115f; } { return 0.13f; } все эти вариации - скорострельность у игрока меньше, чем у NPC, потому закомментил

	bool	CanHolster(void);
	bool	Reload(void);

	bool shouldDropMag; //drop mag
	float dropMagTime; //drop mag
	void DropMag(void); //drop mag

	int		CapabilitiesGet(void) { return bits_CAP_WEAPON_RANGE_ATTACK1; }

	Activity	GetPrimaryAttackActivity(void);

	void	DoImpactEffect(trace_t &tr, int nDamageType);

	virtual const Vector& GetBulletSpread(void)
	{
		static Vector cone;

		cone = VECTOR_CONE_2DEGREES;

		return cone;
	}

	const WeaponProficiencyInfo_t *GetProficiencyValues();

protected:

	float					m_flDelayedFire;
	bool					m_bShotDelayed;
	int						m_nVentPose;

	float m_flSecondaryReloadActivationTime; //new
	float m_flSecondaryReloadDeactivationTime; //new
	float m_flSecondaryEjectTime; //new
	bool m_bSecondaryEjectPending; //new
	float m_flSecondaryEjectTime2; //new
	bool m_bSecondaryEjectPending2; //new

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
private:
	void	SetSkin(int skinNum);
};


#endif	//WEAPONAR2_H