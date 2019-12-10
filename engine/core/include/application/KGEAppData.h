#ifndef KGEAPPLICATION_H
#define KGEAPPLICATION_H

#include <filesystem>
#include <string>

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
               uint32_t applicationVersion);

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
    uint32_t m_applicationVersion{};
};

#endif // KGEAPPLICATION_H
