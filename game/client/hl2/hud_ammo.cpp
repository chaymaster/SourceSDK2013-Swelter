//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "ihudlcd.h"
#include <c_basehlplayer.h>
#include "basehlcombatweapon_shared.h" //added for simple alt reloading

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar extern sde_simple_rifle_bolt;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, CHudNumericDisplay );

public:
	CHudAmmo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset();

	int Simple_Rifle_Bolt;

	bool Annabelle_Round_Chambered;

	bool R357_Round_Chambered;

	void SetAmmo(int ammo, bool playAnimation, C_BaseCombatWeapon *wpn);
	void SetAmmo2(int ammo2, bool playAnimation);
	void SetVehicleAmmo(int ammo, bool playAnimation);
	void SetVehicleAmmo2(int ammo2, bool playAnimation);
	virtual void Paint( void );

protected:
	virtual void OnThink();

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo( C_BasePlayer *player );
	void UpdateVehicleAmmo( C_BasePlayer *player, IClientVehicle *pVehicle );
	
private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;
	int		m_iAmmo;
	int		m_iAmmo2;
	CHudTexture *m_iconPrimaryAmmo;

	int		m_iSimple_Rifle_Bolt;
	bool	m_bAnnabelle_Round_Chambered;
	bool	m_bR357_Round_Chambered;
};

DECLARE_HUDELEMENT( CHudAmmo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo( const char *pElementName ) : BaseClass(NULL, "HudAmmo"), CHudElement( pElementName )
{

	m_iSimple_Rifle_Bolt = 1;

	m_bAnnabelle_Round_Chambered = true;

	m_bR357_Round_Chambered = true;

	SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );

	hudlcd->SetGlobalStat( "(ammo_primary)", "0" );
	hudlcd->SetGlobalStat( "(ammo_secondary)", "0" );
	hudlcd->SetGlobalStat( "(weapon_print_name)", "" );
	hudlcd->SetGlobalStat( "(weapon_name)", "" );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Init( void )
{
	m_iAmmo		= -1;
	m_iAmmo2	= -1;
	
	m_iconPrimaryAmmo = NULL;

	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO");
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"AMMO");
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	BaseClass::Reset();

	m_hCurrentActiveWeapon = NULL;
	m_hCurrentVehicle = NULL;
	m_iAmmo = 0;
	m_iAmmo2 = 0;

	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::UpdatePlayerAmmo( C_BasePlayer *player )
{
	// Clear out the vehicle entity
	m_hCurrentVehicle = NULL;

	C_BaseHLPlayer *HLPlayer = (C_BaseHLPlayer *)player;

	if (HLPlayer)
	{
		Annabelle_Round_Chambered = HLPlayer->Get_Annabelle_Chamber();
		R357_Round_Chambered = HLPlayer->Get_R357_Chamber();
	}

	Simple_Rifle_Bolt = sde_simple_rifle_bolt.GetInt();

	C_BaseCombatWeapon *wpn = GetActiveWeapon();

	const char* ActiveWeaponName = NULL;

	if (wpn)
		ActiveWeaponName = wpn->GetName();

	hudlcd->SetGlobalStat( "(weapon_print_name)", wpn ? wpn->GetPrintName() : " " );
	hudlcd->SetGlobalStat( "(weapon_name)", wpn ? wpn->GetName() : " " );

	if ( !wpn || !player || !wpn->UsesPrimaryAmmo() )
	{
		hudlcd->SetGlobalStat( "(ammo_primary)", "n/a" );
        hudlcd->SetGlobalStat( "(ammo_secondary)", "n/a" );

		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// Get our icons for the ammo types
	m_iconPrimaryAmmo = gWR.GetAmmoIconFromWeapon( wpn->GetPrimaryAmmoType() );

	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}

	hudlcd->SetGlobalStat( "(ammo_primary)", VarArgs( "%d", ammo1 ) );
	hudlcd->SetGlobalStat( "(ammo_secondary)", VarArgs( "%d", ammo2 ) );

	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true, wpn);
		SetAmmo2(ammo2, true);
	}
	else
	{
		// different weapon, change without triggering
		SetAmmo(ammo1, false, wpn);
		SetAmmo2(ammo2, false);

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			SetShouldDisplaySecondaryValue(true);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			SetShouldDisplaySecondaryValue(false);
		}

		//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged"); //prevent the ammo coloring override

		if (ammo1 == 0)
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		else if (!Simple_Rifle_Bolt && (strcmp(ActiveWeaponName, "weapon_357") == 0 || strcmp(ActiveWeaponName, "weapon_annabelle") == 0))
		{
			if ((strcmp(ActiveWeaponName, "weapon_357") == 0 && (!R357_Round_Chambered || wpn->m_bSpecialDrawAnimation || wpn->m_bSpecialHolsterAnimation)) ||
				(strcmp(ActiveWeaponName, "weapon_annabelle") == 0 && (!Annabelle_Round_Chambered || wpn->m_bSpecialDrawAnimation || wpn->m_bSpecialHolsterAnimation)))
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BoltActionNoRoundInChamber");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BoltActionRoundChambered");
		}
		else
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");

		m_hCurrentActiveWeapon = wpn;
	}
}

void CHudAmmo::UpdateVehicleAmmo( C_BasePlayer *player, IClientVehicle *pVehicle )
{
	m_hCurrentActiveWeapon = NULL;
	CBaseEntity *pVehicleEnt = pVehicle->GetVehicleEnt();

	if ( !pVehicleEnt || pVehicle->GetPrimaryAmmoType() < 0 )
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// get the ammo in our clip
	int ammo1 = pVehicle->GetPrimaryAmmoClip();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = pVehicle->GetPrimaryAmmoCount();
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = pVehicle->GetPrimaryAmmoCount();
	}

	if (pVehicleEnt == m_hCurrentVehicle)
	{
		// same weapon, just update counts
		SetVehicleAmmo(ammo1, true);
		SetVehicleAmmo2(ammo2, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetVehicleAmmo(ammo1, false);
		SetVehicleAmmo2(ammo2, false);

		// update whether or not we show the total ammo display
		if (pVehicle->PrimaryAmmoUsesClips())
		{
			SetShouldDisplaySecondaryValue(true);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			SetShouldDisplaySecondaryValue(false);
		}

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentVehicle = pVehicleEnt;
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::OnThink()
{
	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the ammo display counts
//-----------------------------------------------------------------------------
void CHudAmmo::UpdateAmmoDisplays()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	IClientVehicle *pVehicle = player ? player->GetVehicle() : NULL;

	if ( !pVehicle )
	{
		UpdatePlayerAmmo( player );
	}
	else
	{
		UpdateVehicleAmmo( player, pVehicle );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo(int ammo, bool playAnimation, C_BaseCombatWeapon *wpn)
{
	if (!wpn)
		return;

	const char* ActiveWeaponName = wpn->GetName();

	if (ammo != m_iAmmo || Simple_Rifle_Bolt != m_iSimple_Rifle_Bolt ||
		Annabelle_Round_Chambered != m_bAnnabelle_Round_Chambered || R357_Round_Chambered != m_bR357_Round_Chambered)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (!Simple_Rifle_Bolt && (strcmp(ActiveWeaponName, "weapon_357") == 0 || strcmp(ActiveWeaponName, "weapon_annabelle") == 0))
		{
			if ((strcmp(ActiveWeaponName, "weapon_357") == 0 && (!R357_Round_Chambered)) ||
				(strcmp(ActiveWeaponName, "weapon_annabelle") == 0 && (!Annabelle_Round_Chambered)))
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BoltActionNoRoundInChamber");
			else
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("BoltActionRoundChambered");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}
	}

	if (m_iAmmo != ammo)
	{
		m_iAmmo = ammo;
	}

	if (m_iSimple_Rifle_Bolt != Simple_Rifle_Bolt)
	{
		m_iSimple_Rifle_Bolt = Simple_Rifle_Bolt;
	}

	if (m_bAnnabelle_Round_Chambered != Annabelle_Round_Chambered)
	{
		m_bAnnabelle_Round_Chambered = Annabelle_Round_Chambered;
	}

	if (m_bR357_Round_Chambered != R357_Round_Chambered)
	{
		m_bR357_Round_Chambered = R357_Round_Chambered;
	}

	SetDisplayValue(ammo);
}

//-----------------------------------------------------------------------------
// Purpose: Updates 2nd ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo2(int ammo2, bool playAnimation)
{
	if (ammo2 != m_iAmmo2)
	{
		if (ammo2 == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Empty");
		}
		else if (ammo2 < m_iAmmo2)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Decreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Increased");
		}

		m_iAmmo2 = ammo2;
	}

	SetSecondaryValue(ammo2);
}

void CHudAmmo::SetVehicleAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}

		m_iAmmo = ammo;
	}

	SetDisplayValue(ammo);
}
//-----------------------------------------------------------------------------
// Purpose: Updates 2nd ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetVehicleAmmo2(int ammo2, bool playAnimation)
{
	if (ammo2 != m_iAmmo2)
	{
		if (ammo2 == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Empty");
		}
		else if (ammo2 < m_iAmmo2)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Decreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Increased");
		}

		m_iAmmo2 = ammo2;
	}

	SetSecondaryValue(ammo2);
}

//-----------------------------------------------------------------------------
// Purpose: We add an icon into the 
//-----------------------------------------------------------------------------
void CHudAmmo::Paint( void )
{
	BaseClass::Paint();

#ifndef HL2MP
	if ( m_hCurrentVehicle == NULL && m_iconPrimaryAmmo )
	{
		int nLabelHeight;
		int nLabelWidth;
		surface()->GetTextSize( m_hTextFont, m_LabelText, nLabelWidth, nLabelHeight );

		// Figure out where we're going to put this
		int x = text_xpos + ( nLabelWidth - m_iconPrimaryAmmo->Width() ) / 2;
		int y = text_ypos - ( nLabelHeight + ( m_iconPrimaryAmmo->Height() / 2 ) );
		
		m_iconPrimaryAmmo->DrawSelf( x, y, GetFgColor() );
	}
#endif // HL2MP
}
//ConVar	sde_simple_alt_reload2("sde_simple_alt_reload2", "0", FCVAR_CLIENTDLL);
ConVar extern sde_simple_alt_reload;

//-----------------------------------------------------------------------------
// Purpose: Displays the secondary ammunition level
//-----------------------------------------------------------------------------
class CHudSecondaryAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudSecondaryAmmo, CHudNumericDisplay );

public:
	CHudSecondaryAmmo( const char *pElementName ) : BaseClass( NULL, "HudAmmoSecondary" ), CHudElement( pElementName )
	{
		m_iAmmo = -1;

		m_iSimple_Alt_Reload = 0;

		m_bAR1M1_GL_Loaded = false;

		m_bAR2_GL_Loaded = false;

		SetHiddenBits( HIDEHUD_HEALTH | HIDEHUD_WEAPONSELECTION | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT );
	}

	void Init( void )
	{
#ifndef HL2MP
		wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO_ALT");
		if (tempString)
		{
			SetLabelText(tempString);
		}
		else
		{
			SetLabelText(L"ALT");
		}
#endif // HL2MP
	}

	void VidInit( void )
	{
	}

	bool AR1M1_GL_Loaded;

	bool AR2_GL_Loaded;

	int Simple_Alt_Reload;
	
	void SetAmmo(int ammo, C_BaseHLPlayer *HLPlayer, const char* ActiveWeaponName)
	{
		if (Simple_Alt_Reload)
		{
			DevMsg("SDE_HUD_simple on\n");
			if (ammo != m_iAmmo || Simple_Alt_Reload != m_iSimple_Alt_Reload)
			{
				if (ammo == 0)
				{
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryEmptySimple");
				}
				else if (ammo < m_iAmmo)
				{
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryDecreasedSimple");
				}
				else
				{
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryIncreasedSimple");
				}

				// m_iAmmo = ammo; executed below anyway after if-else
			}
		}

		else
		{
			DevMsg("SDE_HUD_simple off\n");
			if (ammo == 0)
			{
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryEmpty");
			}

			if (ammo != m_iAmmo || Simple_Alt_Reload != m_iSimple_Alt_Reload || m_bAR1M1_GL_Loaded != AR1M1_GL_Loaded || m_bAR2_GL_Loaded != AR2_GL_Loaded) // ammo amount changed or the player reloaded the grenade launcher means we call the animation
			{
				if (ammo < m_iAmmo && (/*strcmp(ActiveWeaponName, "weapon_smg1") == 0 ||*/ strcmp(ActiveWeaponName, "weapon_ar1m1") == 0 || strcmp(ActiveWeaponName, "weapon_ar2") == 0))
				{
					// ammo has decreased and current weapon is AR1M1 or AR2 means that its grenade launcher is unloaded
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryDecreasedUnloaded");
					//engine->ClientCmd("testhudanim AmmoSecondaryDecreasedUnloaded");
				}
				else if (ammo < m_iAmmo)
				{
					// ammo has decreased
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryDecreased");
					//engine->ClientCmd("testhudanim AmmoSecondaryDecreased");
				}
				else if (/*(SMG1_GL_Loaded == false && (strcmp(ActiveWeaponName, "weapon_smg1") == 0)) ||*/ (AR1M1_GL_Loaded == false && strcmp(ActiveWeaponName, "weapon_ar1m1") == 0) || (AR2_GL_Loaded == false && strcmp(ActiveWeaponName, "weapon_ar2") == 0))
				{
					// ammunition has increased or tactical grenade launcher reloading had been turned on while active weapon is AR1M1 or AR2 and its grenade launcher is unloaded
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryIncreasedUnloaded");
					//engine->ClientCmd("testhudanim AmmoSecondaryIncreasedUnloaded");
				}
				else
				{
					// ammunition has increased or grenade launcher has been loaded
					g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryIncreased");
					//engine->ClientCmd("testhudanim AmmoSecondaryIncreased");
				}
			}
		}

		if (m_iSimple_Alt_Reload != Simple_Alt_Reload)
		{
			m_iSimple_Alt_Reload = Simple_Alt_Reload;
		}

		if (m_iAmmo != ammo)
		{
			m_iAmmo = ammo;
		}

		if (m_bAR1M1_GL_Loaded != AR1M1_GL_Loaded)
		{
			m_bAR1M1_GL_Loaded = AR1M1_GL_Loaded;
		}

		if (m_bAR2_GL_Loaded != AR2_GL_Loaded)
		{
			m_bAR2_GL_Loaded = AR2_GL_Loaded;
		}

		SetDisplayValue( ammo );
	}

	void Reset()
	{
		// hud reset, update ammo state
		BaseClass::Reset();
		m_iAmmo = 0;
		m_hCurrentActiveWeapon = NULL;
		SetAlpha( 0 );
		UpdateAmmoState();
	}

	virtual void Paint( void )
	{
		BaseClass::Paint();

#ifndef HL2MP
		if ( m_iconSecondaryAmmo )
		{
			int nLabelHeight;
			int nLabelWidth;
			surface()->GetTextSize( m_hTextFont, m_LabelText, nLabelWidth, nLabelHeight );

			// Figure out where we're going to put this
			int x = text_xpos + ( nLabelWidth - m_iconSecondaryAmmo->Width() ) / 2;
			int y = text_ypos - ( nLabelHeight + ( m_iconSecondaryAmmo->Height() / 2 ) );

			m_iconSecondaryAmmo->DrawSelf( x, y, GetFgColor() );
		}
#endif // HL2MP
	}

protected:

	virtual void OnThink()
	{
		// set whether or not the panel draws based on if we have a weapon that supports secondary ammo
		C_BaseCombatWeapon *wpn = GetActiveWeapon();
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		IClientVehicle *pVehicle = player ? player->GetVehicle() : NULL;
		if (!wpn || !player || pVehicle)
		{
			m_hCurrentActiveWeapon = NULL;
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}
		else
		{
			SetPaintEnabled(true);
			SetPaintBackgroundEnabled(true);
		}

		UpdateAmmoState();
	}

	void UpdateAmmoState()
	{
		C_BaseCombatWeapon *wpn = GetActiveWeapon();
		C_BaseHLPlayer *player = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
		const char* ActiveWeaponName = NULL;
		int ammo = 0;

		if (player && wpn && wpn->UsesSecondaryAmmo())
		{
			AR1M1_GL_Loaded = player->Get_AR1M1_GLL();
			AR2_GL_Loaded = player->Get_AR2_GLL();
			Simple_Alt_Reload = sde_simple_alt_reload.GetInt();
			ActiveWeaponName = wpn->GetName();
			ammo = player->GetAmmoCount(wpn->GetSecondaryAmmoType());
			SetAmmo(ammo, player, ActiveWeaponName);
			//SetAmmo(player->GetAmmoCount(wpn->GetSecondaryAmmoType()));
		}

		if ( m_hCurrentActiveWeapon != wpn )
		{

			if ( wpn->UsesSecondaryAmmo() )
			{
				// we've changed to a weapon that uses secondary ammo
				if (Simple_Alt_Reload)
				{
						g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesSecondaryAmmoSimple");
				}
				else if (/*(SMG1_GL_Loaded == false && (strcmp(ActiveWeaponName, "weapon_smg1") == 0)) ||*/ (AR1M1_GL_Loaded == false && strcmp(ActiveWeaponName, "weapon_ar1m1") == 0) || (AR2_GL_Loaded == false && strcmp(ActiveWeaponName, "weapon_ar2") == 0))
				{
						g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesSecondaryAmmoUnloaded");
				}
				else
				{
						g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesSecondaryAmmo");
				}

			}
			else 
			{
				// we've changed away from a weapon that uses secondary ammo
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseSecondaryAmmo");
			}
			m_hCurrentActiveWeapon = wpn;
			
			// Get the icon we should be displaying
			m_iconSecondaryAmmo = gWR.GetAmmoIconFromWeapon( m_hCurrentActiveWeapon->GetSecondaryAmmoType() );
		}
	}
	
private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHudTexture *m_iconSecondaryAmmo;
	int		m_iAmmo;
	int		m_iSimple_Alt_Reload;
	bool	m_bAR1M1_GL_Loaded;
	bool	m_bAR2_GL_Loaded;
};

DECLARE_HUDELEMENT( CHudSecondaryAmmo );

