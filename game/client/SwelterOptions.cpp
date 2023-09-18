#include "cbase.h"
using namespace vgui;
#include <vgui/IVGui.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/RadioButton.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Combobox.h>
#include "ienginevgui.h"



class COptionsSwelter : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsSwelter, vgui::PropertyPage);

public:
	COptionsSwelter(vgui::Panel* parent);
	~COptionsSwelter() {}

	virtual void OnApplyChanges();
	virtual void OnCommand(char const *cmd);

//protected:
//	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

	CheckButton *hintButton;
	CheckButton *altButton;
	CheckButton *muzButton;
	CheckButton *boltButton;
	CheckButton *ccButton;
	CheckButton *crosshairButton;
	CheckButton *holsterButton;
	ComboBox *m_weaponFOV;
	ComboBox *m_ccLang;
	ComboBox *m_reloadingMag;
	ComboBox *m_pCloseCaptionCombo;
	Button	*m_setDefault;
	Button	*m_setClassic;
	Button	*m_setRealism;
	Button	*m_resetAchievements;

	bool b_hintButton;
	bool b_altButton;
	bool b_muzButton;
	bool b_boltButton;
	bool b_ccButton;
	bool b_crosshairButton;
	bool b_holsterButton;
};

COptionsSwelter::COptionsSwelter(vgui::Panel* parent) : PropertyPage(parent, NULL)
{	
	m_pCloseCaptionCombo = new ComboBox(this, "CloseCaptionCheck", 6, false);
	m_pCloseCaptionCombo->AddItem("#GameUI_NoClosedCaptions", NULL);
	m_pCloseCaptionCombo->AddItem("#GameUI_SubtitlesAndSoundEffects", NULL);
	m_pCloseCaptionCombo->AddItem("#GameUI_Subtitles", NULL);

	ConVarRef var0("viewmodel_fov");
	m_weaponFOV = new ComboBox(this, "weaponFOV", 6, false);
	m_weaponFOV->AddItem("#pht_option_weapon_close", NULL);
	m_weaponFOV->AddItem("#pht_option_weapon_medium", NULL);
	m_weaponFOV->AddItem("#pht_option_weapon_far", NULL);


	switch (var0.GetInt())
	{
	case 54:
		m_weaponFOV->ActivateItem(0);
		break;
	case 62:
		m_weaponFOV->ActivateItem(1);
		break;
	case 70:
		m_weaponFOV->ActivateItem(2);
	}

	
	ConVarRef var3("cc_lang");
	m_ccLang = new ComboBox(this, "ccLang", 6, false);
	m_ccLang->AddItem("#pht_option_lang_english", NULL);
	m_ccLang->AddItem("#pht_option_lang_russian", NULL);
	m_ccLang->AddItem("#pht_option_lang_schinese", NULL);
	m_ccLang->AddItem("#pht_option_lang_ukrainian", NULL);
	m_ccLang->AddItem("#pht_option_lang_swedish", NULL);
	m_ccLang->AddItem("#pht_option_lang_korean", NULL);
	m_ccLang->AddItem("#pht_option_lang_spanish", NULL);
	m_ccLang->AddItem("#pht_option_lang_italian", NULL);
	m_ccLang->AddItem("#pht_option_lang_french", NULL);
	m_ccLang->AddItem("#pht_option_lang_japanese", NULL);
	m_ccLang->AddItem("#pht_option_lang_polish", NULL);
	//m_ccLang->AddItem("#pht_option_lang_german", NULL);




	if (FStrEq(var3.GetString(), "english"))
		m_ccLang->ActivateItem(0);
	else if (FStrEq(var3.GetString(), "russian"))
		m_ccLang->ActivateItem(1);
	else if (FStrEq(var3.GetString(), "schinese"))
		m_ccLang->ActivateItem(2);
	else if (FStrEq(var3.GetString(), "ukrainian"))
		m_ccLang->ActivateItem(3);
	else if (FStrEq(var3.GetString(), "swedish"))
		m_ccLang->ActivateItem(4);
	else if (FStrEq(var3.GetString(), "korean"))
		m_ccLang->ActivateItem(5);
	else if (FStrEq(var3.GetString(), "spanish"))
		m_ccLang->ActivateItem(6);
	else if (FStrEq(var3.GetString(), "italian"))
		m_ccLang->ActivateItem(7);
	else if (FStrEq(var3.GetString(), "french"))
		m_ccLang->ActivateItem(8);
	else if (FStrEq(var3.GetString(), "japanese"))
		m_ccLang->ActivateItem(9);
	else if (FStrEq(var3.GetString(), "polish"))
		m_ccLang->ActivateItem(10);
	else if (FStrEq(var3.GetString(), "german"))
		m_ccLang->ActivateItem(110);


	ConVarRef var6("sde_drop_mag");
	m_reloadingMag = new ComboBox(this, "reloadingMag", 6, false);
	m_reloadingMag->AddItem("#pht_option_reloading_0", NULL);
	m_reloadingMag->AddItem("#pht_option_reloading_1", NULL);
	m_reloadingMag->AddItem("#pht_option_reloading_2", NULL);

	switch (var6.GetInt())
	{
	case 0:
		m_reloadingMag->ActivateItem(0);
		break;
	case 1:
		m_reloadingMag->ActivateItem(1);
		break;
	case 2:
		m_reloadingMag->ActivateItem(2);
	}

	ConVarRef var7("sde_simple_rifle_bolt");
	boltButton = new CheckButton(this, "boltButton", "Turn on/off the game bolting the rifles for you");
	if (var7.GetInt() == 0)
	{
		boltButton->SetSelected(true);
		b_boltButton = true;
	}
	else
	{
		boltButton->SetSelected(false);
		b_boltButton = false;
	}
	b_boltButton = boltButton->IsSelected();

	ConVarRef var("sde_weaponhint");
	hintButton = new CheckButton(this, "hintButton", "Turn on/off weapon hud hint");
	if (var.GetInt() == 1)
	{
		hintButton->SetSelected(true);
		b_hintButton = true;
	}
	else
	{
		hintButton->SetSelected(false);
		b_hintButton = false;
	}
	b_hintButton = hintButton->IsSelected();


	ConVarRef var2("sde_simple_alt_reload");
	altButton = new CheckButton(this, "altButton", "Turn on/off tactical alt fire reloading");
	if (var2.GetInt() == 0)
	{
		altButton->SetSelected(true);
		b_altButton = true;
	}
	else
	{
		altButton->SetSelected(false);
		b_altButton = false;
	}
	b_altButton = altButton->IsSelected();

	//close caption old
	/*
	ConVarRef var4("closecaption");
	ConVarRef var5("cc_subtitles");
	ccButton = new CheckButton(this, "ccButton", "Turn on/off closedcaption");
	if (var4.GetInt() == 1)
	{
		ccButton->SetSelected(true);
		b_ccButton = true;
	}
	else
	{
		ccButton->SetSelected(false);
		b_ccButton = false;
	}
	b_ccButton = ccButton->IsSelected();
	*/

	ConVarRef var8("sde_enable_muzzle_flash_light");
	muzButton = new CheckButton(this, "muzButton", "Turn on/off the dynamic lighting based muzzleflash.");
	if (var8.GetInt() == 1)
	{
		muzButton->SetSelected(true);
		b_muzButton = true;
	}
	else
	{
		muzButton->SetSelected(false);
		b_muzButton = false;
	}
	b_muzButton = muzButton->IsSelected();

	// close captions new
	ConVarRef closecaption("closecaption");
	ConVarRef cc_subtitles("cc_subtitles");
	if (closecaption.GetBool())
	{
		if (cc_subtitles.GetBool())
		{
			m_pCloseCaptionCombo->ActivateItem(2);
		}
		else
		{
			m_pCloseCaptionCombo->ActivateItem(1);
		}
	}
	else
	{
		m_pCloseCaptionCombo->ActivateItem(0);
	}


	ConVarRef var9("sde_swelter_crosshair");
	crosshairButton = new CheckButton(this, "crosshairButton", "Switch crosshair style");
	if (var9.GetInt() == 1)
	{
		crosshairButton->SetSelected(false);
		b_crosshairButton = false;
	}
	else
	{
		crosshairButton->SetSelected(true);
		b_crosshairButton = true;
	}
	b_crosshairButton = crosshairButton->IsSelected();

	ConVarRef var11("sde_holster");
	holsterButton = new CheckButton(this, "holsterButton", "turn off holstering anim");
	if (var11.GetInt() == 1)
	{
		holsterButton->SetSelected(false);
		b_holsterButton = false;
	}
	else
	{
		holsterButton->SetSelected(true);
		b_holsterButton = true;
	}
	b_holsterButton = holsterButton->IsSelected();

	//preset buttons
	m_setDefault = new Button(this, "setDefault", "#pht_option_preset_default", this, "setDefault");
	m_setClassic = new Button(this, "setClassic", "#pht_option_preset_classic", this, "setClassic");
	m_setRealism = new Button(this, "setRealism", "#pht_option_preset_realism", this, "setRealism");

	//reset achievements button
	ConVarRef isModDB("sde_ModDB");
	if (isModDB.GetInt() == 0)
	{
		m_resetAchievements = new Button(this, "resetAchievements", "#pht_option_reset", this, "resetAchievements");
		LoadControlSettings("resource/OptionsSwelter.res");
	}
	else
	{
		LoadControlSettings("resource/OptionsSwelter_moddb.res");
	}


	//LoadControlSettings("resource/OptionsSwelter.res");
}

void ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void COptionsSwelter::OnApplyChanges()
{
	switch (m_weaponFOV->GetActiveItem())
	{
	case 0:
		ApplyChangesToConVar("viewmodel_fov", 54);
		break;
	case 1:
		ApplyChangesToConVar("viewmodel_fov", 62);
		break;
	case 2:
		ApplyChangesToConVar("viewmodel_fov", 70);
	}

	ConVarRef var("sde_weaponhint");
	var.SetValue(hintButton->IsSelected());

	ConVarRef var2("sde_simple_alt_reload");
	var2.SetValue(!altButton->IsSelected());

	ConVarRef var7("sde_simple_rifle_bolt");
	var7.SetValue(!boltButton->IsSelected());

	ConVarRef var8("sde_enable_muzzle_flash_light");
	var8.SetValue(muzButton->IsSelected() ? 1 : 0);

	ConVarRef var9("sde_swelter_crosshair");
	ConVarRef var10("hud_quickinfo");
	var9.SetValue(!crosshairButton->IsSelected());
	var10.SetValue(crosshairButton->IsSelected());

	ConVarRef var11("sde_holster");
	var11.SetValue(!holsterButton->IsSelected());

	/* close caption old
	ConVarRef var3("closecaption");
	ConVarRef var4("cc_subtitles");
	var3.SetValue(ccButton->IsSelected());
	var4.SetValue(1);
	*/
	int closecaption_value = 0;

	// close caption new
	ConVarRef cc_subtitles("cc_subtitles");
	switch (m_pCloseCaptionCombo->GetActiveItem())
	{
	default:
	case 0:
		closecaption_value = 0;
		cc_subtitles.SetValue(0);
		break;
	case 1:
		closecaption_value = 1;
		cc_subtitles.SetValue(0);
		break;
	case 2:
		closecaption_value = 1;
		cc_subtitles.SetValue(1);
		break;
	}

	// Stuff the close caption change to the console so that it can be
	//  sent to the server (FCVAR_USERINFO) so that you don't have to restart
	//  the level for the change to take effect.
	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), "closecaption %i\n", closecaption_value);
	engine->ClientCmd_Unrestricted(cmd);
	// close caption end



	switch (m_ccLang->GetActiveItem())
	{
	case 0:
		engine->ClientCmd("cc_lang english\n");
		break;
	case 1:
		engine->ClientCmd("cc_lang russian\n");
		break;
	case 2:
		engine->ClientCmd("cc_lang schinese\n");
		break;
	case 3:
		engine->ClientCmd("cc_lang ukrainian\n");
		break;
	case 4:
		engine->ClientCmd("cc_lang swedish\n");
		break;
	case 5:
		engine->ClientCmd("cc_lang korean\n");
		break;
	case 6:
		engine->ClientCmd("cc_lang spanish\n");
		break;
	case 7:
		engine->ClientCmd("cc_lang italian\n");
		break;
	case 8:
		engine->ClientCmd("cc_lang french\n");
		break;
	case 9:
		engine->ClientCmd("cc_lang japanese\n");
		break;
	case 10:
		engine->ClientCmd("cc_lang polish\n");
		break;
	case 11:
		engine->ClientCmd("cc_lang german\n");
	}



	switch (m_reloadingMag->GetActiveItem())
	{
	case 0:
		ApplyChangesToConVar("sde_drop_mag", 0);
		break;
	case 1:
		ApplyChangesToConVar("sde_drop_mag", 1);
		break;
	case 2:
		ApplyChangesToConVar("sde_drop_mag", 2);
	}
}

void COptionsSwelter::OnCheckButtonChecked(Panel* panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

class CSwelterMenu : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CSwelterMenu, vgui::PropertyDialog);

public:
	CSwelterMenu(vgui::VPANEL parent);
	~CSwelterMenu() {}

	virtual void Activate();


protected:
	virtual void OnTick();
	virtual void OnClose();

private:
	COptionsSwelter* m_SwelterOptions;
};

CSwelterMenu::CSwelterMenu(vgui::VPANEL parent) : BaseClass(NULL, "CSwelterMenu")
{
	SetDeleteSelfOnClose(true);
	ConVarRef isModDB("sde_ModDB");
	if (isModDB.GetInt() == 0)
		SetBounds(0, 0, 512, 476);
	else
		SetBounds(0, 0, 512, 506);
	SetSizeable(false);
	MoveToCenterOfScreen();
	ActivateMinimized();

	SetTitle("#pht_options_title", true);

	m_SwelterOptions = new COptionsSwelter(this);
	AddPage(m_SwelterOptions, "#pht_options_title");

	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(84);
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CSwelterMenu::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);
}

void CSwelterMenu::OnTick()
{
	BaseClass::OnTick();
	if (engine->IsPaused() || engine->IsLevelMainMenuBackground())
		SetVisible(true);
	else
		SetVisible(false);
}

bool isSwelterOptionActive = false;

void CSwelterMenu::OnClose()
{
	BaseClass::OnClose();
	isSwelterOptionActive = false;
}

void COptionsSwelter::OnCommand(char const *cmd)
{
	if (!Q_stricmp(cmd, "setDefault"))
	{
		m_weaponFOV->ActivateItem(0);
		m_reloadingMag->ActivateItem(1);
		hintButton->SetSelected(true);
		altButton->SetSelected(true);
		boltButton->SetSelected(false);
		muzButton->SetSelected(true);
		crosshairButton->SetSelected(false);
		holsterButton->SetSelected(false);
	}
	if (!Q_stricmp(cmd, "setClassic"))
	{
		m_weaponFOV->ActivateItem(1);
		m_reloadingMag->ActivateItem(0);
		hintButton->SetSelected(true);
		altButton->SetSelected(false);
		boltButton->SetSelected(false);
		muzButton->SetSelected(false);
		crosshairButton->SetSelected(true);
		holsterButton->SetSelected(true);
	}
	if (!Q_stricmp(cmd, "setRealism"))
	{
		m_weaponFOV->ActivateItem(2);
		m_reloadingMag->ActivateItem(2);
		hintButton->SetSelected(false);
		altButton->SetSelected(true);
		boltButton->SetSelected(true);
		muzButton->SetSelected(true);
		crosshairButton->SetSelected(false);
		holsterButton->SetSelected(false);
	}
	if (!Q_stricmp(cmd, "resetAchievements"))
	{
		engine->ClientCmd("OpenSwelterReset\n"); 
	}
	else
	{
		BaseClass::OnCommand(cmd);
	}
}


CSwelterMenu* SwelterMenu = NULL;

CON_COMMAND(OpenSwelterOptions, "Turns on SwelterOptions Panel")
{
	VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL_TOOLS);
	if (!isSwelterOptionActive)
	{
		SwelterMenu = new CSwelterMenu(gameToolParent);
		isSwelterOptionActive = true;
	}

	SwelterMenu->Activate();
}
