#pragma once

#include <Project64-core/Settings.h>

class CPluginList
{
public:
    typedef struct
    {
        PLUGIN_INFO Info;
        bool        AboutFunction;
        CPath       FullPath;
        stdstr      FileName;
    } PLUGIN;

public:
    CPluginList(bool bAutoFill = true, bool bUseAdapters = false);
    ~CPluginList();

    bool     LoadList(void);
    bool     LoadList(bool bUseAdapters);
    int      GetPluginCount(void) const;
    const PLUGIN * GetPluginInfo(int indx) const;

private:
    typedef std::vector<PLUGIN>   PluginList;

    PluginList m_PluginList;
    CPath      m_PluginDir;
    bool       m_UseAdapters;

    void AddPluginFromDir(CPath Dir);
    DynLibHandle KailleraPluginInfo(const char* Path, PLUGIN_INFO* PluginInfo);
};
