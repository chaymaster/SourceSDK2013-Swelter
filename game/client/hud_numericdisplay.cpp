//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include <Color.h>
#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudNumericDisplay::CHudNumericDisplay(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_iValue = 0;
	m_LabelText[0] = 0;
	m_iSecondaryValue = 0;
	m_bDisplayValue = true;
	m_bDisplaySecondaryValue = false;
	m_bIndent = false;
	m_bIsTime = false;
}

//-----------------------------------------------------------------------------
// Purpose: Resets values on restore/new map
//-----------------------------------------------------------------------------
void CHudNumericDisplay::Reset()
{
	m_flBlur = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetDisplayValue(int value)
{
	m_iValue = value;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetSecondaryValue(int value)
{
	m_iSecondaryValue = value;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetShouldDisplayValue(bool state)
{
	m_bDisplayValue = state;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetShouldDisplaySecondaryValue(bool state)
{
	m_bDisplaySecondaryValue = state;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetLabelText(const wchar_t *text)
{
	wcsncpy(m_LabelText, text, sizeof(m_LabelText) / sizeof(wchar_t));
	m_LabelText[(sizeof(m_LabelText) / sizeof(wchar_t)) - 1] = 0;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetIndent(bool state)
{
	m_bIndent = state;
}

//-----------------------------------------------------------------------------
// Purpose: data accessor
//-----------------------------------------------------------------------------
void CHudNumericDisplay::SetIsTime(bool state)
{
	m_bIsTime = state;
}

//-----------------------------------------------------------------------------
// Purpose: paints a number at the specified position
//-----------------------------------------------------------------------------
void CHudNumericDisplay::PaintNumbers(HFont font, int xpos, int ypos, int value)
{
	surface()->DrawSetTextFont(font);
	wchar_t unicode[6];
	if ( !m_bIsTime )
	{
		V_snwprintf(unicode, ARRAYSIZE(unicode), L"%d", value);
	}
	else
	{
		int iMinutes = value / 60;
		int iSeconds = value - iMinutes * 60;
#ifdef PORTAL
		// portal uses a normal font for numbers so we need the seperate to be a renderable ':' char
		if ( iSeconds < 10 )
			V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d:0%d", iMinutes, iSeconds );
		else
			V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d:%d", iMinutes, iSeconds );		
#else
		if ( iSeconds < 10 )
			V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d`0%d", iMinutes, iSeconds );
		else
			V_snwprintf( unicode, ARRAYSIZE(unicode), L"%d`%d", iMinutes, iSeconds );
#endif
	}

	// adjust the position to take into account 3 characters
	int charWidth = surface()->GetCharacterWidth(font, '0');
	if (value < 100 && m_bIndent)
	{
		xpos += charWidth;
	}
	if (value < 10 && m_bIndent)
	{
		xpos += charWidth;
	}

	surface()->DrawSetTextPos(xpos, ypos);
	surface()->DrawUnicodeString( unicode );
}

//-----------------------------------------------------------------------------
// Purpose: draws the text
//-----------------------------------------------------------------------------
ConVar sde_hud_adjustment("sde_hud_adjustment", 0, 0, "sde_hud_adjustment");
void CHudNumericDisplay::PaintLabel( void )
{	
	
	if ((m_LabelText[0] == L"+"[0]) || ( m_LabelText[0] == L"*"[0])) //health and suit icon instead text
	{
		
		surface()->GetScreenSize(wide, tall);
		if (sde_hud_adjustment.GetInt() != 0)
		{
			posY = sde_hud_adjustment.GetInt();
			DevMsg("SDE: screen tall: %i \n", tall);
		}
		else
		{
			switch (tall) //display resolution adjustment.
			{
				case 480:
					posY = -23;
					break;
				case 576:
					posY = -28;
					break;
				case 600:
					posY = -30;
					break;
				case 720:
					posY = -35;
					break;
				case 768:
					posY = -38;
					break;
				case 800:
					posY = -39;
					break;
				case 864:
					posY = -43;
					break;
				case 900:
					posY = -45;
					break;
				case 960:
					posY = -47;
					break;
				case 1024:
					posY = -46;
					break;
				case 1050:
					posY = -44;
					break;
				case 1080:
					posY = -40;
					break;
				case 1440:
					posY = -28;
					break;
				case 1600:
					posY = -22;
					break;
				case 2160:
					posY = 0;
					break;
				default:
				posY = -40;
				break;
			}
		}



		posX = text_xpos;
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(GetFgColor());
		surface()->DrawSetTextPos(posX, posY);
		surface()->DrawUnicodeString(m_LabelText);
		
	}
	else
	{
		surface()->DrawSetTextFont(m_hTextFont);
		surface()->DrawSetTextColor(GetFgColor());
		surface()->DrawSetTextPos(text_xpos, text_ypos);
		surface()->DrawUnicodeString(m_LabelText);
	}
}

//-----------------------------------------------------------------------------
// Purpose: renders the vgui panel
//-----------------------------------------------------------------------------
void CHudNumericDisplay::Paint()
{
	if (m_bDisplayValue)
	{
		// draw our numbers
		surface()->DrawSetTextColor(GetFgColor());
		PaintNumbers(m_hNumberFont, digit_xpos, digit_ypos, m_iValue);

		// draw the overbright blur
		for (float fl = m_flBlur; fl > 0.0f; fl -= 1.0f)
		{
			if (fl >= 1.0f)
			{
				PaintNumbers(m_hNumberGlowFont, digit_xpos, digit_ypos, m_iValue);
			}
			else
			{
				// draw a percentage of the last one
				Color col = GetFgColor();
				col[3] *= fl;
				surface()->DrawSetTextColor(col);
				PaintNumbers(m_hNumberGlowFont, digit_xpos, digit_ypos, m_iValue);
			}
		}
	}

	// total ammo
	if (m_bDisplaySecondaryValue)
	{
		surface()->DrawSetTextColor(GetFgColor());
		PaintNumbers(m_hSmallNumberFont, digit2_xpos, digit2_ypos, m_iSecondaryValue);
	}
	if (m_LabelText)
	PaintLabel();
}



