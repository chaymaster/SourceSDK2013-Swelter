//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_weapon__stubs.h"
#include "basehlcombatweapon_shared.h"
#include "c_basehlcombatweapon.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

STUB_WEAPON_CLASS(cycler_weapon, WeaponCycler, C_BaseCombatWeapon);

STUB_WEAPON_CLASS(env_lasersight, LaserSight, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_binoculars, WeaponBinoculars, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_bugbait, WeaponBugBait, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_bugbait2, WeaponBugBait2, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_bugbottle, WeaponBugbottle, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_flaregun, Flaregun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_annabelle, WeaponAnnabelle, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_gauss, WeaponGaussGun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_cubemap, WeaponCubemap, C_BaseCombatWeapon);
STUB_WEAPON_CLASS(weapon_alyxgun, WeaponAlyxGun, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_alyxgun_s, WeaponAlyxGun_s, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_citizenpackage, WeaponCitizenPackage, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_citizensuitcase, WeaponCitizenSuitcase, C_WeaponCitizenPackage);

#ifndef HL2MP

//STUB_WEAPON_CLASS(weapon_stunstick, WeaponStunStick, C_WeaponStunStick);
STUB_WEAPON_CLASS(weapon_ar2, WeaponAR2, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_frag, WeaponFrag, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_handgren, WeaponHandgren, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_rpg, WeaponRPG, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistol, WeaponPistol, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_pistol_m1, WeaponPistol_m1, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_shotgun, WeaponShotgun, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_sos, WeaponSOS, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_smg1, WeaponSMG1, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_smg2, WeaponSMG2, C_HLSelectFireMachineGun);
STUB_WEAPON_CLASS(weapon_357, Weapon357, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_356, Weapon356, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crossbow, WeaponCrossbow, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_slam, Weapon_SLAM, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_crowbar, WeaponCrowbar, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_wrench, WeaponWrench, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_wrench2, WeaponWrench2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_pero, WeaponPero, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_adot, Weaponadot, C_BaseHLCombatWeapon);
#ifdef HL2_EPISODIC
STUB_WEAPON_CLASS(weapon_hopwire, WeaponHopwire, C_BaseHLCombatWeapon);
//STUB_WEAPON_CLASS( weapon_proto1, WeaponProto1, C_BaseHLCombatWeapon );
STUB_WEAPON_CLASS(weapon_ar1, WeaponAR1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_ar1m1, WeaponAR1m1, C_HLMachineGun);
STUB_WEAPON_CLASS(weapon_lopata, WeaponLopata, C_BaseHLCombatWeapon);
STUB_WEAPON_CLASS(weapon_kulak, Weaponkulak, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_kulak2, Weaponkulak2, C_BaseHLBludgeonWeapon);
STUB_WEAPON_CLASS(weapon_glauncher, WeaponGlauncher, C_HLSelectFireMachineGun);
#endif
#ifdef HL2_LOSTCOAST
STUB_WEAPON_CLASS(weapon_oldmanharpoon, WeaponOldManHarpoon, C_WeaponCitizenPackage);
#endif
#endif


