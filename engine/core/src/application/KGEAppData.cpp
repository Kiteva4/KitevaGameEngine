#include "application/KGEAppData.h"

KGEAppData::KGEAppData():
    m_applicationPath{"~/KitevaGameEngine/"},
    m_applicationName{"KGEApp"},
    m_applicationWidth{1024},
    m_applicationHeight{1024},
    m_applicationVersion{1,0,0}
{

}

KGEAppData::KGEAppData(std::filesystem::path applicationPath,
                       std::string applicationName,
                       uint32_t applicationWidth,
                       uint32_t applicationHeight,
                       APP_VERSION applicationVersion) :
    m_applicationPath{applicationPath},
    m_applicationName{applicationName},
    m_applicationWidth{applicationWidth},
    m_applicationHeight{applicationHeight},
    m_applicationVersion{applicationVersion}
{}

std::filesystem::path KGEAppData::applicationPath() const
{
    return m_applicationPath;
}

std::string KGEAppData::applicationName() const
{
    return m_applicationName;
}

uint32_t KGEAppData::applicationWidth() const
{
    return m_applicationWidth;
}

uint32_t KGEAppData::applicationHeight() const
{
    return m_applicationHeight;
}

uint32_t KGEAppData::applicationVersion() const
{
    return KGE_MAKE_VERSION(m_applicationVersion.major,
                            m_applicationVersion.minor,
                            m_applicationVersion.patch);
}
