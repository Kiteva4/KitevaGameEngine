#ifndef KGEAPPLICATION_H
#define KGEAPPLICATION_H

#include <graphic/KGEVulkan.h>
#include <filesystem>
#include <string>

struct APP_VERSION
{
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
};

namespace fs = std::filesystem;
//fs::path appPath = fs::current_path();
//std::cout << "appPath = " << appPath << std::endl;
class KGEAppData
{
public:
    KGEAppData();
    KGEAppData(fs::path applicationPath,
               std::string applicationName,
               uint32_t applicationWidth,
               uint32_t applicationHeight,
               APP_VERSION applicationVersion);

    fs::path applicationPath() const;
    std::string applicationName() const;
    uint32_t applicationWidth() const;
    uint32_t applicationHeight() const;
    uint32_t applicationVersion() const;

private:
    std::string m_applicationPath{};
    std::string m_applicationName{};
    uint32_t m_applicationWidth{};
    uint32_t m_applicationHeight{};
    APP_VERSION m_applicationVersion;
};

#endif // KGEAPPLICATION_H
