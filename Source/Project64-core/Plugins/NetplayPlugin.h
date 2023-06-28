#pragma once
#include <Project64-core/Plugins/PluginBase.h>

class CNetplayPlugin : public CPlugin
{
public:
    CNetplayPlugin(void);
    ~CNetplayPlugin();

    bool Load(const char* FileName);

    bool Initiate(CN64System * System, RenderWindow * Window);

private:
    CNetplayPlugin(const CNetplayPlugin&);
    CNetplayPlugin& operator=(const CNetplayPlugin&);

    virtual int32_t GetDefaultSettingStartRange() const { return FirstNetDefaultSet; }
    virtual int32_t GetSettingStartRange() const { return FirstNetSettings; }
    PLUGIN_TYPE type() { return PLUGIN_TYPE_NETPLAY; }

    bool LoadFunctions(void);
    void UnloadPluginDetails(void);
};
