#include "LightSystem.h"
#include "Player.h"
#include "Map.h"
#include <algorithm>

LightSystem::LightSystem()
    : m_flashlightEnabled(true)
    , m_flashlightBattery(100.0f)
    , m_flashlightRadius(12.0f)         // Увеличил радиус до 12 клеток
    , m_flashlightAngle(1.2f)           // ~70 градусов конус (шире)
    , m_flashlightDrainRate(3.0f)       // 3% в секунду (33 секунды работы - было 20)
    , m_ambientLight(0.03f)             // Минимальный свет (3% - можно различить стены на 0.5-1м)
{
}

void LightSystem::addRoomLights(const Map& map)
{
    clearLights();
    
    const std::vector<Room>& rooms = map.getRooms();
    
    for (const auto& room : rooms)
    {
        // Добавляем источник света в центр каждой комнаты
        float centerX = room.x + room.width / 2.0f;
        float centerY = room.y + room.height / 2.0f;
        
        // Радиус освещения покрывает всю комнату + немного за пределами
        float radius = std::max(room.width, room.height) * 1.5f;
        
        // Цвет света: золотой для выхода, теплый белый для обычных комнат
        sf::Color lightColor = room.isExit ? sf::Color(255, 215, 100) : sf::Color(255, 240, 200);
        
        // Увеличиваем интенсивность света в комнатах (яркие safe зоны)
        m_staticLights.emplace_back(centerX, centerY, radius, 2.0f, lightColor, true);
    }
}

void LightSystem::clearLights()
{
    m_staticLights.clear();
}

void LightSystem::updateFlashlight(float deltaTime, bool isUsing, bool inSafeRoom)
{
    // Зарядка в safe комнате
    if (inSafeRoom && m_flashlightBattery < 100.0f)
    {
        m_flashlightBattery += 20.0f * deltaTime; // Быстрая зарядка 20%/сек
        
        if (m_flashlightBattery > 100.0f)
        {
            m_flashlightBattery = 100.0f;
        }
    }
    // Разрядка при использовании
    else if (isUsing && m_flashlightEnabled && m_flashlightBattery > 0.0f)
    {
        m_flashlightBattery -= m_flashlightDrainRate * deltaTime;
        
        if (m_flashlightBattery < 0.0f)
        {
            m_flashlightBattery = 0.0f;
        }
    }
}

bool LightSystem::hasLineOfSight(float x1, float y1, float x2, float y2, const Map& map) const
{
    // Простая проверка луча между двумя точками
    float dx = x2 - x1;
    float dy = y2 - y1;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    int steps = static_cast<int>(distance * 2.0f); // 2 проверки на клетку
    
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float checkX = x1 + dx * t;
        float checkY = y1 + dy * t;
        
        if (map.isWall(static_cast<int>(checkX), static_cast<int>(checkY)))
        {
            return false; // Стена блокирует свет
        }
    }
    
    return true;
}

float LightSystem::calculateLighting(float x, float y, const Player& player, const Map& map) const
{
    float totalLight = m_ambientLight; // Начинаем с ambient освещения
    
    // 1. Проверяем освещение от статичных источников (комнаты)
    for (const auto& light : m_staticLights)
    {
        float dx = x - light.x;
        float dy = y - light.y;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        if (distance < light.radius)
        {
            // Проверяем прямую видимость
            if (hasLineOfSight(light.x, light.y, x, y, map))
            {
                // Затухание по расстоянию (квадратичное)
                float attenuation = 1.0f - (distance / light.radius);
                attenuation = attenuation * attenuation; // Квадратичное затухание
                
                float lightContribution = light.intensity * attenuation;
                totalLight += lightContribution;
            }
        }
    }
    
    // 2. Проверяем освещение от фонарика игрока
    if (m_flashlightEnabled && m_flashlightBattery > 0.0f)
    {
        float dx = x - player.getX();
        float dy = y - player.getY();
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // ИСПРАВЛЕНИЕ: Ограничиваем дальность фонарика
        if (distance < m_flashlightRadius && distance > 0.01f) // Избегаем деления на 0
        {
            // Вычисляем угол к точке
            float angleToPoint = std::atan2(dy, dx);
            float playerAngle = player.getAngle();
            
            // Нормализуем разницу углов
            float angleDiff = angleToPoint - playerAngle;
            while (angleDiff > 3.14159f) angleDiff -= 2.0f * 3.14159f;
            while (angleDiff < -3.14159f) angleDiff += 2.0f * 3.14159f;
            
            // Проверяем, попадает ли точка в конус фонарика
            if (std::abs(angleDiff) < m_flashlightAngle)
            {
                // Проверяем прямую видимость
                if (hasLineOfSight(player.getX(), player.getY(), x, y, map))
                {
                    // Затухание по расстоянию (более агрессивное)
                    float distanceAttenuation = 1.0f - (distance / m_flashlightRadius);
                    distanceAttenuation = distanceAttenuation * distanceAttenuation * distanceAttenuation; // Кубическое затухание
                    
                    // Затухание по углу (центр луча ярче)
                    float angleAttenuation = 1.0f - (std::abs(angleDiff) / m_flashlightAngle);
                    angleAttenuation = angleAttenuation * angleAttenuation;
                    
                    // Учитываем заряд батареи (мерцание при низком заряде)
                    float batteryMultiplier = m_flashlightBattery / 100.0f;
                    if (m_flashlightBattery < 20.0f)
                    {
                        // Мерцание при низком заряде
                        batteryMultiplier *= 0.5f + 0.5f * std::sin(m_flashlightBattery * 10.0f);
                    }
                    
                    // Увеличиваем интенсивность фонарика для лучшей видимости
                    float flashlightContribution = distanceAttenuation * angleAttenuation * batteryMultiplier * 2.5f;
                    totalLight += flashlightContribution;
                }
            }
        }
    }
    
    // Ограничиваем значение от 0 до 1
    return std::max(0.0f, std::min(1.0f, totalLight));
}
