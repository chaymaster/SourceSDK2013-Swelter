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
//#include "../server/hl2/hl2_player.h"



class COutdatedSwelter : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COutdatedSwelter, vgui::PropertyPage);

public:
	COutdatedSwelter(vgui::Panel* parent);
	~COutdatedSwelter() {}

	virtual void OnApplyChanges();
	virtual void OnCommand(char const *cmd);

	//protected:
	//	virtual void ApplySchemeSettings(vgui::IScheme* pScheme);

private:
	MESSAGE_FUNC_PTR(OnCheckButtonChecked, "CheckButtonChecked", panel);

};

COutdatedSwelter::COutdatedSwelter(vgui::Panel* parent) : PropertyPage(parent, NULL)
{

	LoadControlSettings("resource/OptionsSwelter.res");
}

//void ApplyChangesToConVar(const char *pConVarName, int value)
//{
//	Assert(cvar->FindVar(pConVarName));
//	char szCmd[256];
//	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
//	engine->ClientCmd_Unrestricted(szCmd);
//}

void COutdatedSwelter::OnApplyChanges()
{

}

void COutdatedSwelter::OnCheckButtonChecked(Panel* panel)
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

class CSwelterMenuOutdated : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CSwelterMenuOutdated, vgui::PropertyDialog);

public:
	CSwelterMenuOutdated(vgui::VPANEL parent);
	~CSwelterMenuOutdated() {}

	virtual void Activate();


protected:
	virtual void OnTick();
	virtual void OnClose();

private:
	COutdatedSwelter* m_SwelterOutdated;
};

CSwelterMenuOutdated::CSwelterMenuOutdated(vgui::VPANEL parent) : BaseClass(NULL, "CSwelterMenuOutdated")
{
	SetDeleteSelfOnClose(true);
	SetBounds(0, 0, 640, 512);
	SetSizeable(false);
	MoveToCenterOfScreen();
	ActivateMinimized();

	SetTitle("#pht_warning", true);

	//m_SwelterOutdated = new COutdatedSwelter(this);
	//AddPage(m_SwelterOutdated, "#pht_outdate_title");
	LoadControlSettings("resource/SwelterMapOutdated.res");
	SetApplyButtonVisible(false);
	GetPropertySheet()->SetTabWidth(84);
	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CSwelterMenuOutdated::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);
}

void CSwelterMenuOutdated::OnTick()
{
	BaseClass::OnTick();
	if (engine->IsPaused() || engine->IsLevelMainMenuBackground())
		SetVisible(true);
	else
		SetVisible(false);
}

bool isSwelterOutdateActive = false;

void CSwelterMenuOutdated::OnClose()
{
	engine->ClientCmd("unpause\n");
	BaseClass::OnClose();
	isSwelterOutdateActive = false;
}

void COutdatedSwelter::OnCommand(char const *cmd)
{
}


CSwelterMenuOutdated* SwelterMenuOutdated = NULL;

CON_COMMAND(OpenSwelterOutdate, "Turns on Oudtaded map warning Panel")
{
	VPANEL gameToolParent = enginevgui->GetPanel(PANEL_CLIENTDLL_TOOLS);
	if (!isSwelterOutdateActive)
	{
		SwelterMenuOutdated = new CSwelterMenuOutdated(gameToolParent);
		isSwelterOutdateActive = true;
	}

	SwelterMenuOutdated->Activate();
}
