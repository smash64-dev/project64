#pragma once

#include <Project64-core/Plugin.h>

class COptionNetplayPage :
    public CSettingsPageImpl<COptionNetplayPage>,
    public CSettingsPage
{
    BEGIN_MSG_MAP_EX(COptionNetplayPage)
        COMMAND_ID_HANDLER_EX(IDC_NET_ENABLE, NetplayEnabledChanged)
        COMMAND_HANDLER_EX(NET_LIST, LBN_SELCHANGE, NetplayPluginChanged)
        COMMAND_ID_HANDLER_EX(NET_ABOUT, NetplayPluginAbout)
        COMMAND_ID_HANDLER_EX(IDC_NETPLAY_INCLUDE_UNOFFICIAL, NetplayUnofficialChanged)
        COMMAND_ID_HANDLER_EX(IDC_NETPLAY_FIXED_AUDIO, FixedAudioChanged)
        COMMAND_ID_HANDLER_EX(IDC_NETPLAY_SYNC_AUDIO, SyncAudioChanged)
    END_MSG_MAP()

    enum { IDD = IDD_Settings_Netplay };

public:
    COptionNetplayPage(HWND hParent, const RECT & rcDispay);

    LanguageStringID PageTitle(void) { return TAB_NETPLAY; }
    void             HidePage(void);
    void             ShowPage(void);
    void             ApplySettings(bool UpdateScreen);
    bool             EnableReset(void);
    void             ResetPage(void);

    bool PageAccessible(bool AdvancedMode)
    {
#ifdef NETPLAY
        return CSettingsPageImpl<COptionNetplayPage>::PageAccessible(AdvancedMode);
#else
        return AdvancedMode;
#endif
    }

private:
    void NetplayEnabledChanged(UINT Code, int id, HWND ctl);
    void NetplayUnofficialChanged(UINT Code, int id, HWND ctl);
    void NetplayPluginAbout(UINT /*Code*/, int /*id*/, HWND /*ctl*/) { ShowAboutButton(NET_LIST); }
    void NetplayPluginChanged(UINT /*Code*/, int /*id*/, HWND /*ctl*/) { PluginItemChanged(NET_LIST, NET_ABOUT); }
    void FixCtrlState(void);

    void FixedAudioChanged(UINT Code, int id, HWND ctl);
    void SyncAudioChanged(UINT Code, int id, HWND ctl);

    void ShowAboutButton(int id);
    void PluginItemChanged(int id, int AboutID, bool bSetChanged = true);

    void AddPlugins(int ListId, SettingID Type, PLUGIN_TYPE PluginType);
    void UpdatePlugins(int ListId, SettingID Type, PLUGIN_TYPE PluginType);
    void UpdatePageSettings(void);
    void ApplyComboBoxes(void);
    bool ResetComboBox(CModifiedComboBox & ComboBox, SettingID Type);

    CPartialGroupBox m_NetplayGroup;
    CPluginList m_PluginList;
};
