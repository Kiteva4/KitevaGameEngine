#include <core.h>

Core::Core():
    m_kgeAppData{},
    m_kgeAppication{
        m_kgeAppData.applicationWidth(),
        m_kgeAppData.applicationHeight(),
        m_kgeAppData.applicationName()
        }
{
    m_kgeAppication.Init();
    for (;;) {
        m_kgeAppication.Run();
        break;
    }
}

Core::~Core() {}
