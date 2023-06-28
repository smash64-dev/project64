#include "stdafx.h"
#include <io.h>
#include "PluginList.h"
#include <Project64-core/Plugins/PluginBase.h>

CPluginList::CPluginList(bool bAutoFill /* = true */, bool bUseAdapters) :
    m_PluginDir(g_Settings->LoadStringVal(Directory_Plugin), ""),
    m_UseAdapters(bUseAdapters)
{
    if (bAutoFill)
    {
        LoadList();
    }
}

CPluginList::~CPluginList()
{
}

int CPluginList::GetPluginCount() const
{
    return m_PluginList.size();
}

const CPluginList::PLUGIN * CPluginList::GetPluginInfo(int indx) const
{
    if (indx < 0 || indx >= (int)m_PluginList.size())
    {
        return nullptr;
    }
    return &m_PluginList[indx];
}

bool CPluginList::LoadList()
{
    WriteTrace(TraceUserInterface, TraceDebug, "Start");
    m_PluginList.clear();
    AddPluginFromDir(m_PluginDir);
    WriteTrace(TraceUserInterface, TraceDebug, "Done");
    return true;
}

bool CPluginList::LoadList(bool bUseAdapters)
{
    m_UseAdapters = bUseAdapters;
    return LoadList();
}

void CPluginList::AddPluginFromDir(CPath Dir)
{
    Dir.SetNameExtension("*");
    if (Dir.FindFirst(CPath::FIND_ATTRIBUTE_SUBDIR))
    {
        do
        {
            AddPluginFromDir(Dir);
        } while (Dir.FindNext());
        Dir.UpDirectory();
    }

    Dir.SetNameExtension("*.dll");
    if (Dir.FindFirst())
    {
        HMODULE hLib = nullptr;
        do
        {
            if (hLib)
            {
                FreeLibrary(hLib);
                hLib = nullptr;
            }

            //UINT LastErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS );
            WriteTrace(TraceUserInterface, TraceDebug, "Loading %s", (LPCSTR)Dir);
            hLib = LoadLibrary(stdstr((LPCSTR)Dir).ToUTF16().c_str());
            //SetErrorMode(LastErrorMode);

            if (hLib == nullptr)
            {
                DWORD LoadError = GetLastError();
                WriteTrace(TraceUserInterface, TraceDebug, "Failed to load %s (error: %d)", (LPCSTR)Dir, LoadError);
                continue;
            }

            PLUGIN Plugin = { 0 };
            Plugin.Info.MemoryBswaped = true;

            void(CALL *GetDllInfo) (PLUGIN_INFO * PluginInfo);
            GetDllInfo = (void(CALL *)(PLUGIN_INFO *))GetProcAddress(hLib, "GetDllInfo");
            if (GetDllInfo == nullptr && !m_UseAdapters)
            {
                continue;
            }
            if (m_UseAdapters)
            {
                if (CPluginList::KailleraPluginInfo(Dir, &Plugin.Info) == nullptr)
                {
                    continue;
                }
            }
            else
            {
                GetDllInfo(&Plugin.Info);
            }
            if (!CPlugin::ValidPluginVersion(Plugin.Info))
            {
                continue;
            }

            Plugin.FullPath = Dir;
            Plugin.FileName = stdstr((const char *)Dir).substr(strlen(m_PluginDir));

            if (GetProcAddress(hLib, "DllAbout") != nullptr)
            {
                Plugin.AboutFunction = true;
            }
            m_PluginList.push_back(Plugin);
        } while (Dir.FindNext());

        if (hLib)
        {
            FreeLibrary(hLib);
            hLib = nullptr;
        }
    }
}

DynLibHandle CPluginList::KailleraPluginInfo(const char* Path, PLUGIN_INFO* PluginInfo)
{
    HMODULE hLib = nullptr;

    WriteTrace(TracePlugins, TraceDebug, "Loading %s", Path);
    hLib = LoadLibraryA(Path);

    if (hLib == nullptr)
    {
        DWORD LoadError = GetLastError();
        WriteTrace(TracePlugins, TraceDebug, "Failed to load %s (error: %d)", Path, LoadError);
        return nullptr;
    }

    void(_stdcall * kailleraGetVersion) (void*);
    kailleraGetVersion = (void(_stdcall*)(void*))GetProcAddress(hLib, "_kailleraGetVersion@4");
    if (kailleraGetVersion == nullptr)
    {
        return nullptr;
    }

    char version[100] = { 0 };
    kailleraGetVersion(version);

    PluginInfo->Type = PLUGIN_TYPE_NETPLAY;
    PluginInfo->NormalMemory = false;
    PluginInfo->MemoryBswaped = true;

    if (_stricmp(version, "0.9") == 0)
    {
        PluginInfo->Version = NETPLAY_ADAPTER_TYPE::KAILLERA_0_9;
    }
    else if (_stricmp(version, "") != 0)
    {
        PluginInfo->Version = NETPLAY_ADAPTER_TYPE::KAILLERA_UNKNOWN;
    }
    else
    {
        return nullptr;
    }

    sprintf(PluginInfo->Name, "Kaillera %s (Adapter)", version);
    return hLib;
}