#include "stdafx.h"
#include "NetplayPlugin.h"

CNetplayPlugin::CNetplayPlugin()
{
}

CNetplayPlugin::~CNetplayPlugin()
{
    WriteTrace(TraceNetplayPlugin, TraceDebug, "Start");
    Close(nullptr);
    UnloadPlugin();
    WriteTrace(TraceNetplayPlugin, TraceDebug, "Done");
}

bool CNetplayPlugin::LoadFunctions(void)
{
    return true;
}

bool CNetplayPlugin::Initiate(CN64System * System, RenderWindow * Window)
{
    WriteTrace(TraceNetplayPlugin, TraceDebug, "Start");
    if (m_Initialized)
    {
        Close(Window);
        if (PluginOpened)
        {
            WriteTrace(PluginTraceType(), TraceDebug, "Before plugin opened");
            PluginOpened();
            WriteTrace(PluginTraceType(), TraceDebug, "After plugin opened");
        }
    }

    m_Initialized = true;

    WriteTrace(TraceNetplayPlugin, TraceDebug, "Done (res: %s)", m_Initialized ? "true" : "false");
    return m_Initialized;
}

void CNetplayPlugin::UnloadPluginDetails(void)
{
    WriteTrace(TraceNetplayPlugin, TraceDebug, "Start");
    if (m_LibHandle != nullptr)
    {
        DynamicLibraryClose(m_LibHandle);
        m_LibHandle = nullptr;
    }

    WriteTrace(TraceNetplayPlugin, TraceDebug, "Done");
}
