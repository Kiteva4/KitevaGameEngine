#include "graphic/VulkanWindowControl/WindowsWindowControl.h"
#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
#include "graphic/VulkanWindowControl/MacOSWindowControl.h"
#include "graphic/VulkanWindowControl/GLFWWindowControl.h"

#include "graphic/KGEVulkanApp.h"
#include "graphic/KGEVulkanCore.h"

#include "X11/Xlib.h"
#include "vulkan/vulkan_xcb.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <filesystem>

// Переменная IS_VK_DEBUG будет true если используется debug конфиуграция
// В зависимости от данной переменной некоторое поведение может меняться
#ifdef DEBUG
const bool IS_VK_DEBUG = false;
#else
const bool IS_VK_DEBUG = true;
#endif
// Метод вернет структуру с хендлами текстуры и дескриптора
kge::vkstructs::Texture LoadTextureVk(KGEVulkanCore * renderer, std::filesystem::__cxx11::path pPath);

KGEVulkanApp::KGEVulkanApp(uint32_t width, uint32_t heigh, std::string applicationName):
    m_appWidth{width},
    m_appHeigh{heigh},
    m_applicationName{applicationName},
    m_windowControl{nullptr},
    m_KGEVulkanCore{nullptr}
{

}

KGEVulkanApp::~KGEVulkanApp()
{
    delete m_windowControl;
    //delete m_KGEVulkanCore;
}

void KGEVulkanApp::Init()
{
#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
    // Windows (x64 and x86)
    m_windowControl = new GLFWWindowControl("Window Name");
    //    m_windowControl = new WindowsWindowControl("Window Name");
    //    m_extensions.insert(
    //                m_extensions.end(),
    //                {VK_KHR_SURFACE_EXTENSION_NAME,
    //                 VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    //                 VK_EXT_DEBUG_REPORT_EXTENSION_NAME});
#elif __linux__
    // linux
#elif __APPLE__
    // Mac OS, not sure if this is covered by __posix__ and/or __unix__ though...
    //#define VK_USE_PLATFORM_MACOS_MVK
    m_windowControl = new GLFWWindowControl("Window Name");
#else

#endif

    m_windowControl = new GLFWWindowControl("Window Name");
    m_windowControl->Init(m_appWidth,m_appHeigh);
    //Запрос необходимых glfw расширений
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions{};
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> instanceExtensions (glfwExtensions, glfwExtensions+glfwExtensionCount);

    m_instanceExtensions         = std::move(instanceExtensions);
    m_deviceExtensions           = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    m_validationLayersExtensions = {"VK_LAYER_KHRONOS_validation"};

    // Если это DEBUG конфигурация - запросить еще расширения и слои для валидации
    if(IS_VK_DEBUG){
        std::cout << "DEBUG: debug extensions included!" << std::endl;
        m_instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        m_instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        // m_validationLayersExtensions.push_back("VK_LAYER_LUNARG_standard_validation");
    }

    // Enable surface extensions depending on os
    m_instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#ifdef __ANDROID__
    m_instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(_WIN32)
    m_instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_METAL_EXT)
    m_instanceExtensions.push_back(VK_EXT_METAL_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    m_instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#else
    m_instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif

    m_KGEVulkanCore = new KGEVulkanCore(m_appWidth,
                                        m_appHeigh,
                                        m_applicationName,
                                        m_windowControl,
                                        4,
                                        m_instanceExtensions,
                                        m_deviceExtensions,
                                        m_validationLayersExtensions);

    // Загрузка текстур
    kge::vkstructs::Texture groundTexture = LoadTextureVk(m_KGEVulkanCore, "ground.jpg");
    kge::vkstructs::Texture cubeTexture = LoadTextureVk(m_KGEVulkanCore, "cube.jpg");

/*
    // Пол
    m_KGEVulkanCore->AddPrimitive({
                                      { { -5.0f, 0.0f,  5.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -5.0f, 0.0f,  -5.0f },{ 1.0f, 1.0f, 1.0f },{ 20.0f, 0.0f } },
                                      { { 5.0f,  0.0f,  -5.0f },{ 1.0f, 1.0f, 1.0f },{ 20.0f, 20.0f } },
                                      { { 5.0f,  0.0f,  5.0f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 20.0f } },

                                  }, { 0,1,2,2,3,0 }, &groundTexture, { 0.0f,-0.5f,0.0f }, { 0.0f,0.0f,0.0f });

    // Куб
    m_KGEVulkanCore->AddPrimitive({
                                      { { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { 0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -0.2f, 0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { -0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, 0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, -0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                  }, { 0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8, 12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20 }, &cubeTexture, { 0.0f,-0.3f,-2.0f }, { 0.0f,45.0f,0.0f });

    // Куб
    m_KGEVulkanCore->AddPrimitive({
                                      { { -0.2f, -0.2f,  0.2f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f } },
                                      { { -0.2f,  0.2f,  0.2f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f } },
                                      { {  0.2f,  0.2f,  0.2f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f } },
                                      { {  0.2f, -0.2f,  0.2f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f } },

                                      { { 0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -0.2f, 0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { 0.2f,  0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { -0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { -0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, 0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f,  0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, 0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                      { { 0.2f,  -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 0.0f } },
                                      { { 0.2f,  -0.2f,  -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 0.0f } },
                                      { { -0.2f, -0.2f, -0.2f },{ 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } },
                                      { { -0.2f, -0.2f,  0.2f },{ 1.0f, 1.0f, 1.0f },{ 0.0f, 1.0f } },

                                  },
    { 0,1,2,2,3,0, 4,5,6,6,7,4, 8,9,10,10,11,8, 12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20 },
                                  &cubeTexture,
    { 1.0f,-0.3f,-3.0f }, { 0.0f,0.0f,0.0f });
*/

    // Конфигурация перспективы
    m_KGEVulkanCore->SetCameraPerspectiveSettings(60.0f, 0.1f, 256.0f);

    // Время последнего кадра - время начала цикла
    lastFrameTime = high_resolution_clock::now();
}

void KGEVulkanApp::Run()
{

    while (true)
    {
        std::cout << "app tick" << std::endl;
        // Структура оконного (системного) сообщения
        //MSG msg = {};
        // Если получено какое-то сообщение системы
        //        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        //        {
        //            // Подготовить (трансляция символов) и перенаправить сообщение в оконную процедуру класса окна
        //            TranslateMessage(&msg);
        //            DispatchMessage(&msg);

        //            // Если пришло сообщение WM_QUIT - нужно закрыть прогрумму (оборвать цикл)
        //            if (msg.message == WM_QUIT) {
        //                break;
        //            }
        //        }

        // Если хендл окна не пуст (он может стать пустым при закрытии окна)
        if (m_windowControl) {

            // Время текущего кадра (текущей итерации)
            time_point<high_resolution_clock> currentFrameTime = high_resolution_clock::now();

            // Сколько микросекунд прошло с последней итерации
            // 1 миллисекунда = 1000 микросекунд = 1000000 наносекунд
            int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTime - lastFrameTime).count();

            // Перевести в миллисекунды
            float deltaMs = static_cast<float>(delta) / 1000;

            // Обновить время последней итерации
            lastFrameTime = currentFrameTime;

            // Обновление перемещений камеры с учетом времени кадра
            camera.UpdatePositions(deltaMs);

            // Обновить рендерер и отрисовать кадр
            m_KGEVulkanCore->SetCameraPosition(camera.position.x, camera.position.y, camera.position.z);
            m_KGEVulkanCore->SetCameraRotation(camera.rotation.x, camera.rotation.y, camera.rotation.z);
            m_KGEVulkanCore->Update();
            m_KGEVulkanCore->Draw();
        }
    }

    // Уничтожить рендерер
    delete m_KGEVulkanCore;

    // Выход с кодом 0
    kge::tools::LogMessage("Application closed successfully\n");

}

// Загрузка текстуры
// Метод вернет структуру с хендлами текстуры и дескриптора
kge::vkstructs::Texture LoadTextureVk(KGEVulkanCore * renderer, std::filesystem::path pPath)
{

    int width;        // Ширина загруженного изображения
    int height;       // Высота загруженного изображения
    int channels;     // Кол-во каналов
    int bpp = 4;      // Байт на пиксель

    // Путь к файлу
    std::filesystem::path filename = kge::tools::WorkingDir().concat("textures/" + pPath.string());

    // Получить пиксели (массив байт)
    unsigned char* pixels = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    // Создать текстуру (загрузить пиксели в память устройства)
    kge::vkstructs::Texture result = renderer->CreateTexture(
                pixels,
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height),
                static_cast<uint32_t>(channels),
                static_cast<uint32_t>(bpp));

    // Очистить массив байт
    stbi_image_free(pixels);

    return result;
}
