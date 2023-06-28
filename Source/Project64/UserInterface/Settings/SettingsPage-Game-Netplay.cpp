#include "stdafx.h"

#include "SettingsPage.h"
#include "SettingsPage-Game-Netplay.h"

CGameNetplayPage::CGameNetplayPage(HWND hParent, const RECT& rcDispay)
{
    if (!Create(hParent, rcDispay))
    {
        return;
    }
    SetDlgItemText(IDC_NETPLAY_FIXED_AUDIO, wGS(NETPLAY_FIXED_AUDIO).c_str());
    SetDlgItemText(IDC_NETPLAY_SYNC_AUDIO, wGS(NETPLAY_SYNC_AUDIO).c_str());

    AddModCheckBox(GetDlgItem(IDC_NETPLAY_FIXED_AUDIO), Game_Netplay_ForceFixedAudio);
    AddModCheckBox(GetDlgItem(IDC_NETPLAY_SYNC_AUDIO), Game_Netplay_ForceSyncViaAudio);

    UpdatePageSettings();
}

void CGameNetplayPage::UpdatePageSettings(void)
{
    UpdateCheckBoxes();
    FixCtrlState();
}

void CGameNetplayPage::FixCtrlState(void)
{
    bool bEnabled = g_Settings->LoadBool(Netplay_PluginEnabled);
    ::EnableWindow(GetDlgItem(IDC_NETPLAY_FIXED_AUDIO), bEnabled);
    ::EnableWindow(GetDlgItem(IDC_NETPLAY_SYNC_AUDIO), bEnabled);
}

void CGameNetplayPage::ShowPage()
{
    ShowWindow(SW_SHOW);
}

void CGameNetplayPage::HidePage()
{
    ShowWindow(SW_HIDE);
}

void CGameNetplayPage::ApplySettings(bool UpdateScreen)
{
    CSettingsPageImpl<CGameNetplayPage>::ApplySettings(UpdateScreen);
}

bool CGameNetplayPage::EnableReset(void)
{
    if (CSettingsPageImpl<CGameNetplayPage>::EnableReset()) { return true; }
    return false;
}

void CGameNetplayPage::ResetPage()
{
    CSettingsPageImpl<CGameNetplayPage>::ResetPage();
}

void CGameNetplayPage::FixedAudioChanged(UINT /*Code*/, int id, HWND /*ctl*/)
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

void CGameNetplayPage::SyncAudioChanged(UINT /*Code*/, int id, HWND /*ctl*/)
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