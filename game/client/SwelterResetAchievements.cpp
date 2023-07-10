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



class CResetSwelter : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CResetSwelter, vgui::PropertyPage);

public:
	CResetSwelter(vgui::Panel* parent);
	~CResetSwelter() {}

	
	virtual void OnCommand(char const *cmd);

	

	//protected:
	//	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

};

//CResetSwelter::CResetSwelter(vgui::Panel* parent) : PropertyPage(parent, NULL)
//{
//
//}

//void ApplyChangesToConVar(const char *pConVarName, int value)
//{
//	Assert(cvar->FindVar(pConVarName));
//	char szCmd[256];
//	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
//	engine->ClientCmd_Unrestricted(szCmd);
//}



void CResetSwelter::OnCheckButtonChecked(Panel* panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

class CResetMenuSwelter : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CResetMenuSwelter, vgui::PropertyDialog);

public:
	CResetMenuSwelter(vgui::VPANEL parent);
	~CResetMenuSwelter() {}

	virtual void Activate();
	virtual void ApplyChanges();
	virtual bool OnOK(bool applyOnly);

	CResetSwelter* m_SwelterReset;
	Button	*m_setDefault;

protected:
	virtual void OnTick();
	virtual void OnClose();

private:
	CheckButton *acceptButton;
};

CResetMenuSwelter::CResetMenuSwelter(vgui::VPANEL parent) : BaseClass(NULL, "CResetMenuSwelter")
{
	SetDeleteSelfOnClose(true);
	SetBounds(0, 0, 384, 128);
	SetSizeable(false);
	MoveToCenterOfScreen();
	ActivateMinimized();

	SetTitle("#pht_warning", true);
	//acceptButton = new CheckButton(this, "acceptButton", "Accept reset achievements");
	//acceptButton->SetSelected(false);
	//m_setDefault = new Button(this, "setDefault", "#pht_option_preset_default", this, "setDefault");

	//m_SwelterReset = new CResetSwelter(this);
	//AddPage(m_SwelterReset, "#pht_outdate_title");
	LoadControlSettings("resource/SwelterResetAchievements.res");
	SetApplyButtonVisible(false);
	GetPropertySheet()->SetTabWidth(84);
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CResetMenuSwelter::ApplyChanges()
{
	DevMsg("SDE_achievements_apply \n");
}

bool CResetMenuSwelter::OnOK(bool applyOnly)
{
	// the sheet should have the pages apply changes before we tell the world
	//_propertySheet->ApplyChanges();

	// this should tell anybody who's watching us that we're done
	PostActionSignal(new KeyValues("ApplyChanges"));

	// default to closing
	DevMsg("SDE_achievements_apply \n");
	engine->ClientCmd("achievement_reset_all\n");
	return true;
}

void CResetMenuSwelter::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);
}

void CResetMenuSwelter::OnTick()
{
	BaseClass::OnTick();
	if (engine->IsPaused() || engine->IsLevelMainMenuBackground())
		SetVisible(true);
	else
		SetVisible(false);
}

bool isSwelterResetActive = false;

void CResetMenuSwelter::OnClose()
{
	engine->ClientCmd("unpause\n");
	BaseClass::OnClose();
	isSwelterResetActive = false;
	DevMsg("SDE_achievements_close \n");
}

void CResetSwelter::OnCommand(char const *cmd)
{
	//if (!Q_stricmp(cmd, "setDefault"))
	//{
	//	engine->ClientCmd("unpause\n");
	//	BaseClass::OnClose();
	//	isSwelterResetActive = false;
	//	DevMsg("SDE_achievements_apply \n");
	//}
}


CResetMenuSwelter* SwelterMenuReset = NULL;

CON_COMMAND(OpenSwelterReset, "Open reset achievement window")
{
	VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL_TOOLS);
	if (!isSwelterResetActive)
	{
		SwelterMenuReset = new CResetMenuSwelter(gameToolParent);
		isSwelterResetActive = true;
	}

	SwelterMenuReset->Activate();
}
