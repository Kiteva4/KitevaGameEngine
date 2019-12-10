#include <core.h>

Core::Core():
    m_kgeAppData{},
    m_kgeAppication{
        m_kgeAppData.applicationWidth(),
        m_kgeAppData.applicationHeight()
        }
{
    m_kgeAppication.Init();
    m_kgeAppication.Run();
}

Core::~Core() {}
