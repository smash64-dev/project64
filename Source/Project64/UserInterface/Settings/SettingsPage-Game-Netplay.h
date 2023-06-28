#pragma once

#include <Project64-core/Plugin.h>

class CGameNetplayPage :
    public CSettingsPageImpl<CGameNetplayPage>,
    public CSettingsPage
{
    BEGIN_MSG_MAP_EX(CGameNetplayPage)
        COMMAND_ID_HANDLER_EX(IDC_NETPLAY_FIXED_AUDIO, FixedAudioChanged)
        COMMAND_ID_HANDLER_EX(IDC_NETPLAY_SYNC_AUDIO, SyncAudioChanged)
    END_MSG_MAP()

    enum { IDD = IDD_Settings_GameNetplay };

public:
    CGameNetplayPage(HWND hParent, const RECT & rcDispay);

    LanguageStringID PageTitle(void) { return TAB_NETPLAY; }
    void             HidePage(void);
    void             ShowPage(void);
    void             ApplySettings(bool UpdateScreen);
    bool             EnableReset(void);
    void             ResetPage(void);

    bool PageAccessible(bool AdvancedMode)
    {
#ifdef NETPLAY
        return CSettingsPageImpl<CGameNetplayPage>::PageAccessible(AdvancedMode);
#else
        return AdvancedMode;
#endif
    }

private:
    void FixCtrlState(void);
    void FixedAudioChanged(UINT Code, int id, HWND ctl);
    void SyncAudioChanged(UINT Code, int id, HWND ctl);
    void UpdatePageSettings(void);
};
