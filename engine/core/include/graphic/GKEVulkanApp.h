#pragma once

#include "VulkanWindowControl/IVulkanWindowControl.h"
#include <graphic/KGEVulkanCore.h>

#include <vector>
#include <string>

using std::chrono::high_resolution_clock;
using std::chrono::time_point;

// Структура описывающая модель камеры
struct
{
    // Положение на сцеене и поворот камеры
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);

    // Множитель движения (значения от -1 до 1)
    glm::i8vec3 movement = glm::i8vec3(0, 0, 0);

    // Скорость перемещения и чувствительность мыши
    float movementSpeed = 1.0f;
    float mouseSensitivity = 0.5f;

    // Движется ли камера
    bool IsMoving() const
    {
        return this->movement.x != 0 || this->movement.y != 0 || this->movement.z != 0;
    }

    // Обновление положения
    // float deltaMs - время одного кадра (итерации цикла) за которое будет выполнено перемещение
    void UpdatePositions(float deltaMs)
    {
        if (this->IsMoving())
        {

            // Локальный вектор движения (после нажатия на кнопки управления)
            glm::vec3 movementVectorLocal = glm::vec3(
                static_cast<float>(this->movement.x * ((this->movementSpeed / 1000.0f) * deltaMs)),
                static_cast<float>(this->movement.y * ((this->movementSpeed / 1000.0f) * deltaMs)),
                static_cast<float>(this->movement.z * ((this->movementSpeed / 1000.0f) * deltaMs)));

            // Глобальный вектор движения (с учетом поворота камеры)
            glm::vec3 movementVectorGlobal = glm::vec3(
                (cos(glm::radians(this->rotation.y)) * movementVectorLocal.x) - (sin(glm::radians(this->rotation.y)) * movementVectorLocal.z),
                (cos(glm::radians(this->rotation.x)) * movementVectorLocal.y) + (sin(glm::radians(this->rotation.x)) * movementVectorLocal.z),
                (sin(glm::radians(this->rotation.y)) * movementVectorLocal.x) + (cos(glm::radians(this->rotation.y)) * movementVectorLocal.z));

            // Приращение позиции
            this->position += movementVectorGlobal;
        }
    }
} camera;

// Положение курсора мыши в системе координат окна
struct
{
    int32_t x = 0;
    int32_t y = 0;
} mousePos;

class GKEVulkanApp
{
public:
    GKEVulkanApp(uint32_t width, uint32_t heigh, std::string applicationName);
    ~GKEVulkanApp();

    void Init();
    void Run();

private:
    uint32_t m_appWidth{};
    uint32_t m_appHeigh{};
    std::string m_applicationName;
    IVulkanWindowControl *m_windowControl{};
    KGEVulkanCore *m_KGEVulkanCore{};
    std::vector<const char *> m_instanceExtensions{};
    std::vector<const char *> m_deviceExtensions{};
    std::vector<const char *> m_validationLayersExtensions{};

    // Время последнего кадра (точнее последней итерации)
    time_point<high_resolution_clock> lastFrameTime;
};
