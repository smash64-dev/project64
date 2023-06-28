#include "stdafx.h"

#include "SettingsPage.h"

COptionNetplayPage::COptionNetplayPage(HWND hParent, const RECT & rcDispay) :
    m_PluginList(true, g_Settings->LoadBool(Netplay_SupportUnofficial))
{
    if (!Create(hParent, rcDispay))
    {
        return;
    }
    SetDlgItemText(IDC_NET_ENABLE, wGS(NETPLAY_ENABLE).c_str());
    SetDlgItemText(IDC_NET_NAME, wGS(PLUG_NET).c_str());
    SetDlgItemText(NET_ABOUT, wGS(PLUG_ABOUT).c_str());
    SetDlgItemText(IDC_NETPLAY_INCLUDE_UNOFFICIAL, wGS(NETPLAY_INCLUDE_UNOFFICIAL).c_str());
    SetDlgItemText(IDC_NETPLAY_FIXED_AUDIO, wGS(NETPLAY_FIXED_AUDIO).c_str());
    SetDlgItemText(IDC_NETPLAY_SYNC_AUDIO, wGS(NETPLAY_SYNC_AUDIO).c_str());

    m_NetplayGroup.Attach(GetDlgItem(IDC_NET_NAME));
    AddPlugins(NET_LIST, Plugin_NET_Current, PLUGIN_TYPE_NETPLAY);

    AddModCheckBox(GetDlgItem(IDC_NET_ENABLE), Netplay_PluginEnabled);
    AddModCheckBox(GetDlgItem(IDC_NETPLAY_INCLUDE_UNOFFICIAL), Netplay_SupportUnofficial);
    AddModCheckBox(GetDlgItem(IDC_NETPLAY_FIXED_AUDIO), Netplay_ForceFixedAudio);
    AddModCheckBox(GetDlgItem(IDC_NETPLAY_SYNC_AUDIO), Netplay_ForceSyncViaAudio);

    UpdatePageSettings();
}

void COptionNetplayPage::AddPlugins(int ListId, SettingID Type, PLUGIN_TYPE PluginType)
{
    stdstr Default = g_Settings->LoadStringVal(Type);

    CModifiedComboBox * ComboBox;
    ComboBox = AddModComboBox(GetDlgItem(ListId), Type);
    for (int i = 0, n = m_PluginList.GetPluginCount(); i < n; i++)
    {
        const CPluginList::PLUGIN * Plugin = m_PluginList.GetPluginInfo(i);
        if (Plugin == nullptr)
        {
            continue;
        }
        if (Plugin->Info.Type != PluginType)
        {
            continue;
        }
        if (_stricmp(Default.c_str(), Plugin->FileName.c_str()) == 0)
        {
            ComboBox->SetDefault((WPARAM)Plugin);
        }
        ComboBox->AddItem(stdstr(Plugin->Info.Name).ToUTF16().c_str(), (WPARAM)Plugin);
    }
}

void COptionNetplayPage::UpdatePlugins(int ListId, SettingID Type, PLUGIN_TYPE PluginType)
{
    bool bUseAdapters = (SendDlgItemMessage(IDC_NETPLAY_INCLUDE_UNOFFICIAL, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_PluginList.LoadList(bUseAdapters);

    CModifiedComboBox* ComboBox = nullptr;
    ComboBox = AddModComboBox(GetDlgItem(ListId), Type);
    ComboBox->ResetContent();
    AddPlugins(ListId, Type, PluginType);
}

void COptionNetplayPage::ShowAboutButton(int id)
{
    CModifiedComboBox * ComboBox = nullptr;
    for (ComboBoxList::iterator cb_iter = m_ComboBoxList.begin(); cb_iter != m_ComboBoxList.end(); cb_iter++)
    {
        if ((int)(cb_iter->second->GetMenu()) != id)
        {
            continue;
        }
        ComboBox = cb_iter->second;
        break;
    }
    if (ComboBox == nullptr)
    {
        return;
    }
    int index = ComboBox->GetCurSel();
    if (index == CB_ERR)
    {
        return;
    }

    const CPluginList::PLUGIN ** PluginPtr = (const CPluginList::PLUGIN **)ComboBox->GetItemDataPtr(index);
    if (PluginPtr == nullptr)
    {
        return;
    }

    const CPluginList::PLUGIN * Plugin = *PluginPtr;
    if (Plugin == nullptr)
    {
        return;
    }

    // Load the plugin
    UINT LastErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    HMODULE hLib = LoadLibrary(stdstr((const char *)(Plugin->FullPath)).ToUTF16().c_str());
    SetErrorMode(LastErrorMode);
    if (hLib == nullptr)
    {
        return;
    }

    // Get DLL about
    void(CALL *DllAbout) (HWND hWnd);
    DllAbout = (void(CALL *)(HWND))GetProcAddress(hLib, "DllAbout");

    // Call the function from the DLL
    DllAbout(m_hWnd);

    FreeLibrary(hLib);
}

void COptionNetplayPage::PluginItemChanged(int id, int AboutID, bool bSetChanged)
{
    CModifiedComboBox * ComboBox = nullptr;
    for (ComboBoxList::iterator cb_iter = m_ComboBoxList.begin(); cb_iter != m_ComboBoxList.end(); cb_iter++)
    {
        if ((int)(cb_iter->second->GetMenu()) != id)
        {
            continue;
        }
        ComboBox = cb_iter->second;
        break;
    }
    if (ComboBox == nullptr)
    {
        return;
    }

    int index = ComboBox->GetCurSel();
    if (index == CB_ERR)
    {
        return;
    }
    const CPluginList::PLUGIN ** PluginPtr = (const CPluginList::PLUGIN **)ComboBox->GetItemDataPtr(index);
    if (PluginPtr)
    {
        const CPluginList::PLUGIN * Plugin = *PluginPtr;
        if (Plugin)
        {
            ::EnableWindow(GetDlgItem(AboutID), Plugin->AboutFunction);
        }
    }

    if (bSetChanged)
    {
        ComboBox->SetChanged(true);
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
    }
}

void COptionNetplayPage::UpdatePageSettings(void)
{
    UpdateCheckBoxes();
    for (ComboBoxList::iterator cb_iter = m_ComboBoxList.begin(); cb_iter != m_ComboBoxList.end(); cb_iter++)
    {
        CModifiedComboBox * ComboBox = cb_iter->second;
        stdstr SelectedValue;

        ComboBox->SetChanged(g_Settings->LoadStringVal(cb_iter->first, SelectedValue));
        for (int i = 0, n = ComboBox->GetCount(); i < n; i++)
        {
            const CPluginList::PLUGIN ** PluginPtr = (const CPluginList::PLUGIN **)ComboBox->GetItemDataPtr(i);
            if (PluginPtr == nullptr)
            {
                continue;
            }
            const CPluginList::PLUGIN * Plugin = *PluginPtr;
            if (Plugin == nullptr)
            {
                continue;
            }
            if (_stricmp(SelectedValue.c_str(), Plugin->FileName.c_str()) != 0)
            {
                continue;
            }
            ComboBox->SetDefault((WPARAM)Plugin);
        }
    }
    FixCtrlState();
}

void COptionNetplayPage::FixCtrlState(void)
{
    bool bEnabled = (SendDlgItemMessage(IDC_NET_ENABLE, BM_GETCHECK, 0, 0) == BST_CHECKED);
    bool bBasicMode = g_Settings->LoadBool(UserInterface_BasicMode);
    bool bGameRunning = g_Settings->LoadBool(GameRunning_CPU_Running);

    if (bBasicMode)
    {
        SetDlgItemText(IDC_NET_ENABLE, wGS(NETPLAY_ENABLE_ADVANCED).c_str());
    }
    else
    {
        SetDlgItemText(IDC_NET_ENABLE, wGS(NETPLAY_ENABLE).c_str());
    }
    ::EnableWindow(GetDlgItem(IDC_NET_ENABLE), !bGameRunning && !bBasicMode);
    ::EnableWindow(GetDlgItem(NET_LIST), bEnabled);
    ::EnableWindow(GetDlgItem(IDC_NETPLAY_INCLUDE_UNOFFICIAL), !bBasicMode && bEnabled);
    ::EnableWindow(GetDlgItem(IDC_NETPLAY_FIXED_AUDIO), bEnabled);
    ::EnableWindow(GetDlgItem(IDC_NETPLAY_SYNC_AUDIO), bEnabled);

    if (bEnabled)
    {
        PluginItemChanged(NET_LIST, NET_ABOUT, false);
    }
    else
    {
        ::EnableWindow(GetDlgItem(NET_ABOUT), bEnabled);
    }
}

void COptionNetplayPage::HidePage()
{
    ShowWindow(SW_HIDE);
}

void COptionNetplayPage::ShowPage()
{
    ShowWindow(SW_SHOW);
}

void COptionNetplayPage::ApplySettings(bool UpdateScreen)
{
    CSettingsPageImpl<COptionNetplayPage>::ApplySettings(UpdateScreen);
}

bool COptionNetplayPage::EnableReset(void)
{
    if (CSettingsPageImpl<COptionNetplayPage>::EnableReset()) { return true; }
    return false;
}

void COptionNetplayPage::ResetPage()
{
    CSettingsPageImpl<COptionNetplayPage>::ResetPage();
}

void COptionNetplayPage::ApplyComboBoxes(void)
{
    for (ComboBoxList::iterator cb_iter = m_ComboBoxList.begin(); cb_iter != m_ComboBoxList.end(); cb_iter++)
    {
        CModifiedComboBox * ComboBox = cb_iter->second;
        if (ComboBox->IsChanged())
        {
            int index = ComboBox->GetCurSel();
            if (index == CB_ERR)
            {
                return;
            }

            const CPluginList::PLUGIN ** PluginPtr = (const CPluginList::PLUGIN **)ComboBox->GetItemDataPtr(index);
            if (PluginPtr == nullptr)
            {
                return;
            }

            const CPluginList::PLUGIN * Plugin = *PluginPtr;

            g_Settings->SaveString(cb_iter->first, Plugin->FileName.c_str());
        }
        if (ComboBox->IsReset())
        {
            g_Settings->DeleteSetting(cb_iter->first);
            ComboBox->SetReset(false);
        }
    }
}

bool COptionNetplayPage::ResetComboBox(CModifiedComboBox & ComboBox, SettingID Type)
{
    if (!ComboBox.IsChanged())
    {
        return false;
    }

    ComboBox.SetReset(true);
    stdstr Value = g_Settings->LoadDefaultString(Type);
    for (int i = 0, n = ComboBox.GetCount(); i < n; i++)
    {
        const CPluginList::PLUGIN ** PluginPtr = (const CPluginList::PLUGIN **)ComboBox.GetItemDataPtr(i);
        if (PluginPtr == nullptr)
        {
            continue;
        }

        const CPluginList::PLUGIN * Plugin = *PluginPtr;
        if (Plugin->FileName != Value)
        {
            continue;
        }
        ComboBox.SetCurSel(i);
        return true;
    }
    return false;
}

void COptionNetplayPage::NetplayEnabledChanged(UINT /*Code*/, int id, HWND /*ctl*/)
{
    for (ButtonList::iterator iter = m_ButtonList.begin(); iter != m_ButtonList.end(); iter++)
    {
        CModifiedButton* Button = iter->second;
        if ((int)Button->GetMenu() != id)
        {
            continue;
        }
        if ((Button->GetCheck() & BST_CHECKED) == 0)
        {
            if (!g_Notify->AskYesNoQuestion(g_Lang->GetString(MSG_NETPLAY_ENABLED_MSG).c_str()))
            {
                Button->SetCheck(BST_CHECKED);
                return;
            }
        }
        Button->SetChanged(true);
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
        FixCtrlState();
        break;
    }
}

void COptionNetplayPage::NetplayUnofficialChanged(UINT /*Code*/, int id, HWND /*ctl*/)
{
    for (ButtonList::iterator iter = m_ButtonList.begin(); iter != m_ButtonList.end(); iter++)
    {
        CModifiedButton* Button = iter->second;
        if ((int)Button->GetMenu() != id)
        {
            continue;
        }
        if ((Button->GetCheck() & BST_CHECKED) != 0)
        {
            if (!g_Notify->AskYesNoQuestion(g_Lang->GetString(MSG_NETPLAY_INCLUDE_UNOFFICIAL_MSG).c_str()))
            {
                Button->SetCheck(BST_CHECKED);
                return;
            }
        }
        Button->SetChanged(true);
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
        UpdatePlugins(NET_LIST, Plugin_NET_Current, PLUGIN_TYPE_NETPLAY);
        break;
    }
}

void COptionNetplayPage::FixedAudioChanged(UINT /*Code*/, int id, HWND /*ctl*/)
{
    for (ButtonList::iterator iter = m_ButtonList.begin(); iter != m_ButtonList.end(); iter++)
    {
        CModifiedButton* Button = iter->second;
        if ((int)Button->GetMenu() != id)
        {
            continue;
        }
        if ((Button->GetCheck() & BST_CHECKED) == 0)
        {
            if (!g_Notify->AskYesNoQuestion(g_Lang->GetString(MSG_NETPLAY_FIXED_AUDIO_MSG).c_str()))
            {
                Button->SetCheck(BST_CHECKED);
                return;
            }
        }
        Button->SetChanged(true);
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
        break;
    }
}

void COptionNetplayPage::SyncAudioChanged(UINT /*Code*/, int id, HWND /*ctl*/)
{
    for (ButtonList::iterator iter = m_ButtonList.begin(); iter != m_ButtonList.end(); iter++)
    {
        CModifiedButton* Button = iter->second;
        if ((int)Button->GetMenu() != id)
        {
            continue;
        }
        if ((Button->GetCheck() & BST_CHECKED) == 0)
        {
            if (!g_Notify->AskYesNoQuestion(g_Lang->GetString(MSG_NETPLAY_SYNC_AUDIO_MSG).c_str()))
            {
                Button->SetCheck(BST_CHECKED);
                return;
            }
        }
        Button->SetChanged(true);
        SendMessage(GetParent(), PSM_CHANGED, (WPARAM)m_hWnd, 0);
        break;
    }
}