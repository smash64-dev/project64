#include <Settings/Settings.h>

#if defined(LEGACY)
#include <Settings/mINI.h>
#include <map>
#endif

#include <Common/Platform.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_WIN32)
#define EXPORT      extern "C" __declspec(dllexport)
#define CALL        __cdecl
#else
#define EXPORT      extern "C" __attribute__((visibility("default")))
#define CALL
#endif

CNotification g_NotifyLocal;
CNotification * g_Notify = &g_NotifyLocal;

enum SettingLocation
{
    SettingType_ConstString = 0,
    SettingType_ConstValue = 1,
    SettingType_CfgFile = 2,
    SettingType_Registry = 3,
    SettingType_RelativePath = 4,
    TemporarySetting = 5,
    SettingType_RomDatabase = 6,
    SettingType_CheatSetting = 7,
    SettingType_GameSetting = 8,
    SettingType_BoolVariable = 9,
    SettingType_NumberVariable = 10,
    SettingType_StringVariable = 11,
    SettingType_SelectedDirectory = 12,
    SettingType_RdbSetting = 13,
};

enum SettingDataType
{
    Data_DWORD = 0, Data_String = 1, Data_CPUTYPE = 2, Data_SelfMod = 3, Data_OnOff = 4, Data_YesNo = 5, Data_SaveChip = 6
};

typedef struct
{
    uint32_t dwSize;
    int DefaultStartRange;
    int SettingStartRange;
    int MaximumSettings;
    int NoDefault;
    int DefaultLocation;
    void * handle;

    unsigned int(*GetSetting)      (void * handle, int ID);
    const char * (*GetSettingSz)    (void * handle, int ID, char * Buffer, int BufferLen);
    void(*SetSetting)      (void * handle, int ID, unsigned int Value);
    void(*SetSettingSz)    (void * handle, int ID, const char * Value);
    void(*RegisterSetting) (void * handle, int ID, int DefaultID, SettingDataType Type,
        SettingLocation Location, const char * Category, const char * DefaultStr, uint32_t Value);
    void(*UseUnregisteredSetting) (int ID);
} PLUGIN_SETTINGS;

typedef struct
{
    unsigned int(*FindSystemSettingId) (void * handle, const char * Name);
} PLUGIN_SETTINGS2;

typedef struct
{
    void(*FlushSettings) (void * handle);
} PLUGIN_SETTINGS3;

typedef struct
{
    typedef void(*SettingChangedFunc)(void *);

    void(*RegisterChangeCB)(void * handle, int ID, void * Data, SettingChangedFunc Func);
    void(*UnregisterChangeCB)(void * handle, int ID, void * Data, SettingChangedFunc Func);
} PLUGIN_SETTINGS_NOTIFICATION;

typedef struct
{
    void(*DisplayError)(const char * Message);
    void(*FatalError)(const char * Message);
    void(*DisplayMessage)(int DisplayTime, const char * Message);
    void(*DisplayMessage2)(const char * Message);
    void(*BreakPoint)(const char * FileName, int32_t LineNumber);
} PLUGIN_NOTIFICATION;

static PLUGIN_SETTINGS  g_PluginSettings;
static PLUGIN_SETTINGS2 g_PluginSettings2;
static PLUGIN_SETTINGS3 g_PluginSettings3;
static PLUGIN_SETTINGS_NOTIFICATION g_PluginSettingsNotification;
static PLUGIN_NOTIFICATION g_PluginNotification;
static bool g_PluginInitilized = false;
static char g_PluginSettingName[300];

EXPORT void SetSettingInfo(PLUGIN_SETTINGS * info);
EXPORT void SetSettingInfo2(PLUGIN_SETTINGS2 * info);
EXPORT void SetSettingInfo3(PLUGIN_SETTINGS3 * info);
EXPORT void SetPluginNotification(PLUGIN_NOTIFICATION * info);

#if defined(LEGACY)
// mimics the parameters from both RegisterSettings
typedef struct
{
    short SettingID;
    SETTING_DATA_TYPE Type;
    const char* Name;
    const char* Category;
    unsigned int DefaultDW;
    const char* DefaultStr;
    short DefaultID;
} REGISTERED_SETTING;

static std::map<short, REGISTERED_SETTING> g_RegisteredSettings;
static mINI::INIFile g_IniFile("legacy.cfg");
static mINI::INIStructure g_Ini;
#endif

EXPORT void SetSettingInfo(PLUGIN_SETTINGS * info)
{
    g_PluginSettings = *info;
    g_PluginInitilized = true;
    info->UseUnregisteredSetting = UseUnregisteredSetting;
}

EXPORT void SetSettingInfo2(PLUGIN_SETTINGS2 * info)
{
    g_PluginSettings2 = *info;
}

EXPORT void SetSettingInfo3(PLUGIN_SETTINGS3 * info)
{
    g_PluginSettings3 = *info;
}

EXPORT void SetSettingNotificationInfo(PLUGIN_SETTINGS_NOTIFICATION * info)
{
    g_PluginSettingsNotification = *info;
}

EXPORT void SetPluginNotification(PLUGIN_NOTIFICATION * info)
{
    g_PluginNotification = *info;
}

void SettingsInitialize(void)
{
#if defined(LEGACY)
    if (!g_PluginInitilized)
        g_RegisteredSettings.clear();

    g_IniFile.read(g_Ini);
#endif
    return;
}

int32_t SettingsInitilized(void)
{
    return g_PluginInitilized;
}

void SetModuleName(const char * Name)
{
    _snprintf(g_PluginSettingName, sizeof(g_PluginSettingName), "%s", Name);
}

void RegisterSetting(short SettingID, SETTING_DATA_TYPE Type, const char * Name, const char * Category,
    unsigned int DefaultDW, const char * DefaultStr)
{
#if defined(LEGACY)
    REGISTERED_SETTING setting;
    setting.SettingID = SettingID;
    setting.Type = Type;
    setting.Name = Name;
    setting.Category = Category;
    setting.DefaultDW = DefaultDW;
    setting.DefaultStr = DefaultStr;
    setting.DefaultID = 0;  // unused

    g_RegisteredSettings.insert(std::pair<short, REGISTERED_SETTING>(SettingID, setting));
    return;
#else
    if (g_PluginSettings.RegisterSetting == nullptr)
    {
        return;
    }
    int DefaultID = g_PluginSettings.NoDefault;
    SettingLocation Location = (SettingLocation)g_PluginSettings.DefaultLocation;
    char FullCategory[400];
    if (Category && Category[0] != 0)
    {
        _snprintf(FullCategory, sizeof(FullCategory), "%s\\%s", g_PluginSettingName, Category);
    }
    else
    {
        _snprintf(FullCategory, sizeof(FullCategory), "%s", g_PluginSettingName);
    }

    switch (Type)
    {
    case Data_DWORD_Game:
    case Data_String_Game:
        Location = SettingType_GameSetting;
        break;
    case Data_DWORD_RDB:
    case Data_String_RDB:
        Location = SettingType_RomDatabase;
        break;
    case Data_DWORD_RDB_Setting:
    case Data_String_RDB_Setting:
        Location = SettingType_RdbSetting;
        break;
    case Data_DWORD_General:
    case Data_String_General:
    default:
        Location = (SettingLocation)g_PluginSettings.DefaultLocation;
        break;
    }

    switch (Type)
    {
    case Data_DWORD_Game:
        g_PluginSettings.RegisterSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange,
            g_PluginSettings.NoDefault, Data_DWORD, Location, FullCategory, Name, DefaultDW);
        break;
    case Data_DWORD_General:
    case Data_DWORD_RDB:
    case Data_DWORD_RDB_Setting:
        if (DefaultDW != 0)
        {
            // Create default
            DefaultID = SettingID + g_PluginSettings.DefaultStartRange;
            g_PluginSettings.RegisterSetting(g_PluginSettings.handle, DefaultID, g_PluginSettings.NoDefault,
                Data_DWORD, SettingType_ConstValue, g_PluginSettingName, "", DefaultDW);
        }

        g_PluginSettings.RegisterSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange,
            DefaultID, Data_DWORD, Location, FullCategory, Name, 0);
        break;
    case Data_String_General:
    case Data_String_Game:
    case Data_String_RDB:
    case Data_String_RDB_Setting:
        if (DefaultStr != nullptr && strlen(DefaultStr) > 0)
        {
            // Create default
            DefaultID = SettingID + g_PluginSettings.DefaultStartRange;
            g_PluginSettings.RegisterSetting(g_PluginSettings.handle, DefaultID, g_PluginSettings.NoDefault,
                Data_String, SettingType_ConstString, g_PluginSettingName, DefaultStr, 0);
        }

        g_PluginSettings.RegisterSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange,
            DefaultID, Data_String, Location, FullCategory, Name, 0);
        break;
    }
#endif
}

void RegisterSetting2(short SettingID, SETTING_DATA_TYPE Type, const char * Name, const char * Category, short DefaultID)
{
#if defined(LEGACY)
    REGISTERED_SETTING setting;
    setting.SettingID = SettingID;
    setting.Type = Type;
    setting.Name = Name;
    setting.Category = Category;
    setting.DefaultDW = 0;  // unused
    setting.DefaultStr = 0; // unused
    setting.DefaultID = DefaultID;

    g_RegisteredSettings.insert(std::pair<short, REGISTERED_SETTING>(SettingID, setting));
    return;
#else
    SettingLocation Location = (SettingLocation)g_PluginSettings.DefaultLocation;
    char FullCategory[400];
    if (Category && Category[0] != 0)
    {
        _snprintf(FullCategory, sizeof(FullCategory), "%s\\%s", g_PluginSettingName, Category);
    }
    else
    {
        _snprintf(FullCategory, sizeof(FullCategory), "%s", g_PluginSettingName);
    }

    switch (Type)
    {
    case Data_DWORD_Game:
    case Data_String_Game:
        Location = SettingType_GameSetting;
        break;
    case Data_DWORD_RDB:
    case Data_String_RDB:
        Location = SettingType_RomDatabase;
        break;
    case Data_DWORD_RDB_Setting:
    case Data_String_RDB_Setting:
        Location = SettingType_RdbSetting;
        break;
    case Data_DWORD_General:
    case Data_String_General:
    default:
        Location = (SettingLocation)g_PluginSettings.DefaultLocation;
        break;
    }

    switch (Type)
    {
    case Data_DWORD_Game:
    case Data_DWORD_General:
    case Data_DWORD_RDB:
    case Data_DWORD_RDB_Setting:
        g_PluginSettings.RegisterSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange,
            DefaultID + g_PluginSettings.SettingStartRange, Data_DWORD, Location, FullCategory, Name, 0);
        break;
    case Data_String_General:
    case Data_String_Game:
    case Data_String_RDB:
    case Data_String_RDB_Setting:
        g_PluginSettings.RegisterSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange,
            DefaultID + g_PluginSettings.SettingStartRange, Data_String, Location, FullCategory, Name, 0);
        break;
    }
#endif
}

short FindSystemSettingId(const char * Name)
{
#if defined(LEGACY)
    // adapted from CSettings::FindSetting
    for (std::map<short, REGISTERED_SETTING>::iterator iter = g_RegisteredSettings.begin(); iter != g_RegisteredSettings.end(); iter++)
    {
        REGISTERED_SETTING Setting = iter->second;
        if (_stricmp(Setting.Name, Name) == 0)
            return Setting.SettingID;
    }
    return 0;
#else
    if (g_PluginSettings2.FindSystemSettingId && g_PluginSettings.handle)
    {
        return (short)g_PluginSettings2.FindSystemSettingId(g_PluginSettings.handle, Name);
    }
    return 0;
#endif
}

void FlushSettings(void)
{
#if defined(LEGACY)
    g_IniFile.generate(g_Ini);
#else
    if (g_PluginSettings3.FlushSettings && g_PluginSettings.handle)
    {
        g_PluginSettings3.FlushSettings(g_PluginSettings.handle);
    }
#endif
}

unsigned int GetSetting(short SettingID)
{
#if defined(LEGACY)
    std::map<short, REGISTERED_SETTING>::iterator itr = g_RegisteredSettings.find(SettingID);
    if (itr == g_RegisteredSettings.end()) {
        return 0;
    }

    REGISTERED_SETTING setting = itr->second;
    if (g_Ini.get(setting.Category).has(setting.Name))
        return (unsigned int)std::stoi(g_Ini.get(setting.Category).get(setting.Name));
    else
        return setting.DefaultDW;
#else
    if (g_PluginSettings.GetSetting == nullptr)
    {
        return 0;
    }
    return g_PluginSettings.GetSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange);
#endif
}

unsigned int GetSystemSetting(short SettingID)
{
#if defined(LEGACY)
    return GetSetting(SettingID);
#else
    return g_PluginSettings.GetSetting(g_PluginSettings.handle, SettingID);
#endif
}

const char * GetSettingSz(short SettingID, char * Buffer, int BufferLen)
{
#if defined(LEGACY)
    std::map<short, REGISTERED_SETTING>::iterator itr = g_RegisteredSettings.find(SettingID);
    if (itr == g_RegisteredSettings.end()) {
        return "";
    }

    REGISTERED_SETTING setting = itr->second;
    std::string value;

    if (g_Ini.get(setting.Category).has(setting.Name))
        value = g_Ini.get(setting.Category).get(setting.Name).c_str();
    else
        value = setting.DefaultStr;

    std::strcpy(Buffer, value.c_str());
    return Buffer;
#else
    return g_PluginSettings.GetSettingSz(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange, Buffer, BufferLen);
#endif
}

const char * GetSystemSettingSz(short SettingID, char * Buffer, int BufferLen)
{
#if defined(LEGACY)
    return GetSettingSz(SettingID, Buffer, BufferLen);
#else
    if (g_PluginSettings.GetSettingSz == nullptr)
    {
        return "";
    }
    return g_PluginSettings.GetSettingSz(g_PluginSettings.handle, SettingID, Buffer, BufferLen);
#endif
}

void SetSetting(short SettingID, unsigned int Value)
{
#if defined(LEGACY)
    std::map<short, REGISTERED_SETTING>::iterator itr = g_RegisteredSettings.find(SettingID);
    if (itr == g_RegisteredSettings.end()) {
        return;
    }

    REGISTERED_SETTING setting = itr->second;
    g_Ini[setting.Category].set({ {setting.Name, std::to_string(Value)} });
#else
    g_PluginSettings.SetSetting(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange, Value);
#endif
}

void SetSettingSz(short SettingID, const char * Value)
{
#if defined(LEGACY)
    std::map<short, REGISTERED_SETTING>::iterator itr = g_RegisteredSettings.find(SettingID);
    if (itr == g_RegisteredSettings.end()) {
        return;
    }

    REGISTERED_SETTING setting = itr->second;
    g_Ini[setting.Category].set({ {setting.Name, Value} });
#else
    g_PluginSettings.SetSettingSz(g_PluginSettings.handle, SettingID + g_PluginSettings.SettingStartRange, Value);
#endif
}

void SetSystemSetting(short SettingID, unsigned int Value)
{
#if defined(LEGACY)
    SetSetting(SettingID, Value);
#else
    g_PluginSettings.SetSetting(g_PluginSettings.handle, SettingID, Value);
#endif
}

void SetSystemSettingSz(short SettingID, const char * Value)
{
#if defined(LEGACY)
    SetSettingSz(SettingID, Value);
#else
    g_PluginSettings.SetSettingSz(g_PluginSettings.handle, SettingID, Value);
#endif
}

void SettingsRegisterChange(bool SystemSetting, int SettingID, void * Data, SettingChangedFunc Func)
{
    if (g_PluginSettingsNotification.RegisterChangeCB && g_PluginSettings.handle)
    {
        g_PluginSettingsNotification.RegisterChangeCB(g_PluginSettings.handle, SettingID + (SystemSetting ? 0 : g_PluginSettings.SettingStartRange), Data, Func);
    }
}

void SettingsUnregisterChange(bool SystemSetting, int SettingID, void * Data, SettingChangedFunc Func)
{
    if (g_PluginSettingsNotification.UnregisterChangeCB && g_PluginSettings.handle)
    {
        g_PluginSettingsNotification.UnregisterChangeCB(g_PluginSettings.handle, SettingID + (SystemSetting ? 0 : g_PluginSettings.SettingStartRange), Data, Func);
    }
}

void CNotification::DisplayError(const char * Message)
{
    if (g_PluginNotification.BreakPoint != nullptr)
    {
        g_PluginNotification.DisplayError(Message);
    }
}

void CNotification::FatalError(const char * Message)
{
    if (g_PluginNotification.BreakPoint != nullptr)
    {
        g_PluginNotification.FatalError(Message);
    }
}

void CNotification::DisplayMessage(int DisplayTime, const char * Message)
{
    if (g_PluginNotification.BreakPoint != nullptr)
    {
        g_PluginNotification.DisplayMessage(DisplayTime, Message);
    }
}

void CNotification::DisplayMessage2(const char * Message)
{
    if (g_PluginNotification.BreakPoint != nullptr)
    {
        g_PluginNotification.DisplayMessage2(Message);
    }
}

void CNotification::BreakPoint(const char * FileName, int LineNumber)
{
    if (g_PluginNotification.BreakPoint != nullptr)
    {
        g_PluginNotification.BreakPoint(FileName, LineNumber);
    }
}