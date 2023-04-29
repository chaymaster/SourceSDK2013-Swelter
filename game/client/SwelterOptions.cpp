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

//protected:
//	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

	CheckButton *hintButton;
	CheckButton *altButton;
	CheckButton *ccButton;
	ComboBox *m_weaponFOV;
	ComboBox *m_ccLang;
	ComboBox *m_reloadingMag;

	bool b_hintButton;
	bool b_altButton;
	bool b_ccButton;
};

COptionsSwelter::COptionsSwelter(vgui::Panel* parent) : PropertyPage(parent, NULL)
{
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
	//m_ccLang->AddItem("#pht_option_lang_german", NULL);
	//m_ccLang->AddItem("#pht_option_lang_schinese", NULL);


	/*
	if (printf("%s", var3.GetString()) == printf("english"))
		m_ccLang->ActivateItem(0);
	else if (printf("%s", var3.GetString()) == printf("russian"))
		m_ccLang->ActivateItem(1);
	else if (printf("%s", var3.GetString()) == printf("german"))
		m_ccLang->ActivateItem(2);
	else if (printf("%s", var3.GetString()) == printf("schinese"))
		m_ccLang->ActivateItem(3);
	*/



	if (FStrEq(var3.GetString(), "english"))
		m_ccLang->ActivateItem(0);
	else if (FStrEq(var3.GetString(), "russian"))
		m_ccLang->ActivateItem(1);
	else if (FStrEq(var3.GetString(), "german"))
		m_ccLang->ActivateItem(2);
	else if (FStrEq(var3.GetString(), "schinese"))
		m_ccLang->ActivateItem(3);


	//ConVarRef var6("viewmodel_fov");
	//m_reloadingMag = new ComboBox(this, "reloadingMag", 6, false);
	//m_reloadingMag->AddItem("#pht_option_reloading_0", NULL);
	//m_reloadingMag->AddItem("#pht_option_reloading_1", NULL);
	//m_reloadingMag->AddItem("#pht_option_reloading_2", NULL);

	//switch (var6.GetInt())
	//{
	//case 0:
	//	m_weaponFOV->ActivateItem(0);
	//	break;
	//case 1:
	//	m_weaponFOV->ActivateItem(1);
	//	break;
	//case 2:
	//	m_weaponFOV->ActivateItem(2);
	//}



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






	LoadControlSettings("resource/OptionsSwelter.res");
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

	ConVarRef var3("closecaption");
	ConVarRef var4("cc_subtitles");
	var3.SetValue(ccButton->IsSelected());
	var4.SetValue(1);

	switch (m_ccLang->GetActiveItem())
	{
	case 0:
		engine->ClientCmd("cc_lang english\n");
		break;
	case 1:
		engine->ClientCmd("cc_lang russian\n");
		break;
	case 2:
		engine->ClientCmd("cc_lang german\n"); 
		break;
	case 3:
		engine->ClientCmd("cc_lang schinese\n");
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
	SetBounds(0, 0, 512, 406);
	SetSizeable(false);
	MoveToCenterOfScreen();

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
