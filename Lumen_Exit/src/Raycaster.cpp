#include "Raycaster.h"
#include "Player.h"
#include "Map.h"
#include "LightSystem.h"
#include <cmath>
#include <algorithm>

const float PI = 3.14159265359f;

Raycaster::Raycaster(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_fov(PI / 3.0f) // 60 градусов
{
}

Raycaster::RayHit Raycaster::castRay(float rayAngle, const Player& player, const Map& map)
{
    RayHit hit;
    hit.distance = 1000.0f;
    hit.hitVertical = false;
    
    float rayDirX = std::cos(rayAngle);
    float rayDirY = std::sin(rayAngle);
    
    // DDA Algorithm
    float posX = player.getX();
    float posY = player.getY();
    
    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);
    
    float deltaDistX = (rayDirX == 0) ? 1e30f : std::abs(1.0f / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30f : std::abs(1.0f / rayDirY);
    
    int stepX, stepY;
    float sideDistX, sideDistY;
    
    if (rayDirX < 0)
    {
        stepX = -1;
        sideDistX = (posX - mapX) * deltaDistX;
    }
    else
    {
        stepX = 1;
        sideDistX = (mapX + 1.0f - posX) * deltaDistX;
    }
    
    if (rayDirY < 0)
    {
        stepY = -1;
        sideDistY = (posY - mapY) * deltaDistY;
    }
    else
    {
        stepY = 1;
        sideDistY = (mapY + 1.0f - posY) * deltaDistY;
    }
    
    // Бросаем луч до столкновения со стеной
    bool hitWall = false;
    int side = 0; // 0 = вертикальная стена, 1 = горизонтальная
    
    while (!hitWall && hit.distance > 0.1f)
    {
        if (sideDistX < sideDistY)
        {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        }
        else
        {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }
        
        if (map.isWall(mapX, mapY))
        {
            hitWall = true;
        }
    }
    
    // Вычисляем расстояние до стены и точную позицию попадания
    if (side == 0)
    {
        hit.distance = (mapX - posX + (1 - stepX) / 2) / rayDirX;
        hit.hitVertical = true;
        hit.hitX = posX + hit.distance * rayDirX;
        hit.hitY = posY + hit.distance * rayDirY;
    }
    else
    {
        hit.distance = (mapY - posY + (1 - stepY) / 2) / rayDirY;
        hit.hitVertical = false;
        hit.hitX = posX + hit.distance * rayDirX;
        hit.hitY = posY + hit.distance * rayDirY;
    }
    
    hit.mapX = mapX;
    hit.mapY = mapY;
    
    return hit;
}

void Raycaster::render(sf::RenderWindow& window, const Player& player, const Map& map, const LightSystem& lightSystem)
{
    // Создаем буфер для отрисовки (оптимизация)
    sf::VertexArray floorCeiling(sf::Quads);
    sf::VertexArray wallSlices(sf::Quads);
    
    // Отрисовка каждого вертикального столбца экрана
    for (int x = 0; x < m_screenWidth; ++x)
    {
        // Вычисляем угол луча для текущего столбца
        float cameraX = 2.0f * x / static_cast<float>(m_screenWidth) - 1.0f;
        float rayAngle = player.getAngle() + std::atan(cameraX * std::tan(m_fov / 2.0f));
        
        // Бросаем луч
        RayHit hit = castRay(rayAngle, player, map);
        
        // Исправляем fish-eye эффект
        float correctedDistance = hit.distance * std::cos(rayAngle - player.getAngle());
        
        // Вычисляем высоту стены на экране
        int wallHeight = static_cast<int>(m_screenHeight / correctedDistance);
        
        // Вычисляем позицию стены по вертикали
        int drawStart = (m_screenHeight - wallHeight) / 2;
        int drawEnd = drawStart + wallHeight;
        
        // НОВОЕ: Вычисляем среднюю освещенность вдоль луча (volumetric)
        // Берем несколько точек вдоль луча и усредняем освещенность
        float totalLighting = 0.0f;
        int samples = 5; // Фиксированное количество сэмплов для стабильности
        
        float rayDirX = std::cos(rayAngle);
        float rayDirY = std::sin(rayAngle);
        
        // Ограничиваем максимальную дистанцию для сэмплинга
        float maxSampleDist = std::min(correctedDistance, 15.0f);
        
        for (int i = 0; i < samples; ++i)
        {
            float t = (static_cast<float>(i) / static_cast<float>(samples - 1)) * maxSampleDist;
            float sampleX = player.getX() + rayDirX * t;
            float sampleY = player.getY() + rayDirY * t;
            
            float lighting = lightSystem.calculateLighting(sampleX, sampleY, player, map);
            
            // Добавляем fog эффект - дальние точки темнее
            float fogFactor = 1.0f - (t / maxSampleDist);
            fogFactor = fogFactor * fogFactor; // Квадратичное затухание
            
            totalLighting += lighting * (0.5f + 0.5f * fogFactor); // Fog влияет на 50%
        }
        
        float avgLighting = totalLighting / static_cast<float>(samples);
        
        // Дополнительно учитываем освещенность точки попадания (стена может быть ярче)
        float wallLighting = lightSystem.calculateLighting(hit.hitX, hit.hitY, player, map);
        
        // Комбинируем: 60% объемный свет + 40% освещение стены
        float finalLighting = avgLighting * 0.6f + wallLighting * 0.4f;
        
        // НОВОЕ: Добавляем дистанционный туман ТОЛЬКО для ambient света
        // Свет от комнат и фонарика не затухает туманом, только ambient
        // Без фонарика ambient видно только 0.8 метра, с фонариком - 6 метров
        float fogDistance = lightSystem.isFlashlightEnabled() && lightSystem.getFlashlightBattery() > 0.0f ? 6.0f : 0.8f;
        
        // Вычисляем ambient компонент (минимальный свет без источников)
        float ambientComponent = 0.03f; // 3% ambient
        
        // Применяем туман только к ambient компоненту
        float distanceFog = 1.0f;
        if (correctedDistance > fogDistance)
        {
            distanceFog = 0.0f; // Ambient полностью исчезает
        }
        else
        {
            float fogRatio = correctedDistance / fogDistance;
            distanceFog = 1.0f - (fogRatio * fogRatio * fogRatio * fogRatio);
        }
        
        // Разделяем освещение на ambient и источники света
        float lightSourceComponent = std::max(0.0f, finalLighting - ambientComponent);
        float foggedAmbient = ambientComponent * distanceFog;
        
        // Итоговое освещение: источники света (без тумана) + ambient (с туманом)
        finalLighting = lightSourceComponent + foggedAmbient;
        
        // Базовая яркость стены с учетом освещения
        float baseBrightness = finalLighting;
        
        // Разный оттенок для вертикальных и горизонтальных стен
        float wallBrightness = baseBrightness * (hit.hitVertical ? 1.0f : 0.8f);
        
        int colorValue = static_cast<int>(wallBrightness * 255.0f);
        colorValue = std::max(0, std::min(255, colorValue));
        sf::Color wallColor(colorValue, colorValue, colorValue);
        
        // Добавляем вертикальную линию стены в буфер
        float xPos = static_cast<float>(x);
        float yStart = static_cast<float>(drawStart);
        float yEnd = static_cast<float>(drawEnd);
        
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yStart), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yEnd), wallColor));
        
        // Отрисовка пола и потолка с объемным освещением
        // Потолок
        if (drawStart > 0)
        {
            // Используем объемное освещение для потолка (темнее чем стены)
            int ceilingValue = static_cast<int>(avgLighting * 20.0f); // Еще темнее
            
            // ВАЖНО: Применяем туман только к ambient части
            float lightSourceCeiling = std::max(0.0f, avgLighting - ambientComponent);
            float foggedAmbientCeiling = ambientComponent * distanceFog;
            float finalCeilingLight = lightSourceCeiling + foggedAmbientCeiling;
            
            ceilingValue = static_cast<int>(finalCeilingLight * 20.0f);
            
            sf::Color ceilingColor(ceilingValue, ceilingValue, ceilingValue);
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, 0.0f), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, 0.0f), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yStart), ceilingColor));
        }
        
        // Пол
        if (drawEnd < m_screenHeight)
        {
            // Используем объемное освещение для пола
            float lightSourceFloor = std::max(0.0f, avgLighting - ambientComponent);
            float foggedAmbientFloor = ambientComponent * distanceFog;
            float finalFloorLight = lightSourceFloor + foggedAmbientFloor;
            
            int floorValue = static_cast<int>(finalFloorLight * 30.0f); // Чуть светлее потолка
            
            sf::Color floorColor(floorValue, floorValue, floorValue);
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yEnd), floorColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), floorColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, static_cast<float>(m_screenHeight)), floorColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, static_cast<float>(m_screenHeight)), floorColor));
        }
    }
    
    // Рисуем пол и потолок сначала
    window.draw(floorCeiling);
    // Потом стены поверх
    window.draw(wallSlices);
}
