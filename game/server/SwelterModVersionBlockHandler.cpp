#include "cbase.h"
#include "gameinterface.h"
#include "SwelterModVersionBlockHandler.h"

static short MYBLOCKHANDLER_SAVE_RESTORE_VERSION = 1;
extern ConVar sde_mod_version;
extern ConVar sde_mod_version_display;
class CSwelterModVersionBlockHandler : public CDefSaveRestoreBlockHandler
{
public:
	const char *GetBlockName()
	{
		return "MyBlockHandler";
	}

	//---------------------------------

	void Save( ISave *pSave )
	{
		pSave->StartBlock( "MyBlockHandler" );

		int	Data = sde_mod_version.GetInt();
		

		pSave->WriteInt( &Data );

		pSave->EndBlock();
	}

	//---------------------------------

	void WriteSaveHeaders( ISave *pSave )
	{
		pSave->WriteShort( &MYBLOCKHANDLER_SAVE_RESTORE_VERSION );
	}
	
	//---------------------------------

	void ReadRestoreHeaders( IRestore *pRestore )
	{
		// No reason why any future version shouldn't try to retain backward compatability. The default here is to not do so.
		short version;
		pRestore->ReadShort( &version );
		// only load if version matches and if we are loading a game, not a transition
		m_fDoLoad = ( ( version == MYBLOCKHANDLER_SAVE_RESTORE_VERSION ) && 
			( ( MapLoad_LoadGame == gpGlobals->eLoadType ) || ( MapLoad_NewGame == gpGlobals->eLoadType )  ) 
		);
	}

	//---------------------------------

	void Restore( IRestore *pRestore, bool createPlayers )
	{
		//CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		if ( m_fDoLoad )
		{
			pRestore->StartBlock();

			int data = pRestore->ReadInt();

			if (sde_mod_version.GetInt() == NULL || data != sde_mod_version.GetInt())
			{
				Color ConsoleColor(100, 255, 100, 255);
				ConColorMsg(ConsoleColor, (char*)"\n  SDE_OUTDATED_MAP_LOADED \n\n");
				sde_mod_version_display.SetValue(1);
			}

			
			pRestore->EndBlock();
		}
	}

private:
	bool m_fDoLoad;
};

//-----------------------------------------------------------------------------

CSwelterModVersionBlockHandler g_SwelterModVersionBlockHandler;

//-------------------------------------

ISaveRestoreBlockHandler *SwelterModVersionBlockHandler()
{
	return &g_SwelterModVersionBlockHandler;
}