#include "Raycaster.h"
#include "Player.h"
#include "Map.h"
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
    
    // Вычисляем расстояние до стены
    if (side == 0)
    {
        hit.distance = (mapX - posX + (1 - stepX) / 2) / rayDirX;
        hit.hitVertical = true;
    }
    else
    {
        hit.distance = (mapY - posY + (1 - stepY) / 2) / rayDirY;
        hit.hitVertical = false;
    }
    
    hit.mapX = mapX;
    hit.mapY = mapY;
    
    return hit;
}

void Raycaster::render(sf::RenderWindow& window, const Player& player, const Map& map)
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
        
        // Затемнение в зависимости от расстояния
        float brightness = 1.0f / (1.0f + correctedDistance * correctedDistance * 0.05f);
        brightness = std::max(0.0f, std::min(1.0f, brightness));
        
        // Разный оттенок для вертикальных и горизонтальных стен
        int colorValue = static_cast<int>(brightness * (hit.hitVertical ? 255 : 180));
        sf::Color wallColor(colorValue, colorValue, colorValue);
        
        // Добавляем вертикальную линию стены в буфер
        float xPos = static_cast<float>(x);
        float yStart = static_cast<float>(drawStart);
        float yEnd = static_cast<float>(drawEnd);
        
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yStart), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), wallColor));
        wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yEnd), wallColor));
        
        // Отрисовка пола и потолка
        // Потолок (темно-серый)
        if (drawStart > 0)
        {
            sf::Color ceilingColor(30, 30, 35); // Темный потолок
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, 0.0f), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, 0.0f), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), ceilingColor));
            floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yStart), ceilingColor));
        }
        
        // Пол (чуть светлее потолка)
        if (drawEnd < m_screenHeight)
        {
            sf::Color floorColor(40, 40, 45); // Темный пол
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
