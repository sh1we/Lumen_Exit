#include "Raycaster.h"
#include "Player.h"
#include "Map.h"
#include "LightSystem.h"
#include "Config.h"
#include "MathUtils.h"
#include <cmath>
#include <algorithm>

const float PI = MathUtils::PI;

Raycaster::Raycaster(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_fov(PI / 3.0f)  // 60 deg
    , m_floorCeiling(sf::Quads)
    , m_wallSlices(sf::Quads)
    , m_lightingQuality(LightingQuality::HIGH)
    , m_lastLighting(0.0f)
{
    // preallocate so we don't thrash memory every frame
    m_floorCeiling.resize(screenWidth * 8);
    m_wallSlices.resize(screenWidth * 4);
}

Raycaster::RayHit Raycaster::castRay(float rayAngle, const Player& player, const Map& map)
{
    RayHit hit;
    hit.distance = 1000.0f;
    hit.hitVertical = false;
    
    float rayDirX, rayDirY;
    MathUtils::sincos_fast(rayAngle, rayDirY, rayDirX);
    
    // DDA - the classic wolfenstein way
    float posX = player.getX();
    float posY = player.getY();
    
    int mapX = static_cast<int>(posX);
    int mapY = static_cast<int>(posY);
    
    // avoid div by zero with a big number
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
    
    bool hitWall = false;
    int side = 0;  // 0 = vertical, 1 = horizontal
    
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
    m_floorCeiling.clear();
    m_wallSlices.clear();
    
    for (int x = 0; x < m_screenWidth; ++x)
    {
        float cameraX = 2.0f * x / static_cast<float>(m_screenWidth) - 1.0f;
        float rayAngle = player.getAngle() + std::atan(cameraX * std::tan(m_fov / 2.0f));
        
        RayHit hit = castRay(rayAngle, player, map);
        
        // fish-eye fix
        float angleDiff = rayAngle - player.getAngle();
        
        // keep angle in [-PI, PI] or things get weird
        while (angleDiff > PI) angleDiff -= 2.0f * PI;
        while (angleDiff < -PI) angleDiff += 2.0f * PI;
        
        float correctedDistance = hit.distance * std::cos(angleDiff);
        
        if (correctedDistance < 0.1f)
            correctedDistance = 0.1f;
        
        int wallHeight = static_cast<int>(m_screenHeight / correctedDistance);
        
        // sanity check
        if (wallHeight > m_screenHeight * 10)
            wallHeight = m_screenHeight * 10;
        
        int drawStart = (m_screenHeight - wallHeight) / 2;
        int drawEnd = drawStart + wallHeight;
        
        // adaptive sampling - more samples up close where it matters
        int samples;
        bool useInterpolation = false;
        
        switch (m_lightingQuality)
        {
            case LightingQuality::LOW:
                samples = 2;
                useInterpolation = (x % 2 == 1);  // skip every other ray
                break;
            case LightingQuality::MEDIUM:
                samples = 3;
                break;
            case LightingQuality::HIGH:
            default:
                samples = correctedDistance < 4.0f ? 5 : 3;
                break;
        }
        
        float avgLighting;
        
        if (useInterpolation && x > 0)
        {
            avgLighting = m_lastLighting;
        }
        else
        {
            // volumetric lighting - sample along the ray
            float totalLighting = 0.0f;
            
            float rayDirX = std::cos(rayAngle);
            float rayDirY = std::sin(rayAngle);
            
            float maxSampleDist = std::min(correctedDistance, 15.0f);
            
            for (int i = 0; i < samples; ++i)
            {
                float t = (static_cast<float>(i) / static_cast<float>(samples - 1)) * maxSampleDist;
                float sampleX = player.getX() + rayDirX * t;
                float sampleY = player.getY() + rayDirY * t;
                
                float lighting = lightSystem.calculateLighting(sampleX, sampleY, player, map);
                
                // fog falloff
                float fogFactor = 1.0f - (t / maxSampleDist);
                fogFactor = fogFactor * fogFactor;
                
                totalLighting += lighting * (0.5f + 0.5f * fogFactor);
            }
            
            avgLighting = totalLighting / static_cast<float>(samples);
            
            // blend with wall hit point lighting
            float wallLighting = lightSystem.calculateLighting(hit.hitX, hit.hitY, player, map);
            avgLighting = avgLighting * 0.6f + wallLighting * 0.4f;
            
            m_lastLighting = avgLighting;
        }
        
        // fog only affects ambient, not light sources (so you can't cheat with monitor brightness)
        float fogDistance = lightSystem.isFlashlightEnabled() && lightSystem.getFlashlightBattery() > 0.0f ? 6.0f : 0.8f;
        
        float ambientComponent = 0.03f;  // 3% base visibility
        
        float distanceFog = 1.0f;
        if (correctedDistance > fogDistance)
        {
            distanceFog = 0.0f;
        }
        else
        {
            float fogRatio = correctedDistance / fogDistance;
            distanceFog = 1.0f - (fogRatio * fogRatio * fogRatio * fogRatio);
        }
        
        float lightSourceComponent = std::max(0.0f, avgLighting - ambientComponent);
        float foggedAmbient = ambientComponent * distanceFog;
        
        float finalLighting = lightSourceComponent + foggedAmbient;
        float baseBrightness = finalLighting;
        
        // horizontal walls slightly darker for depth
        float wallBrightness = baseBrightness * (hit.hitVertical ? 1.0f : 0.8f);
        
        int colorValue = static_cast<int>(wallBrightness * 255.0f);
        colorValue = std::max(0, std::min(255, colorValue));
        sf::Color wallColor(colorValue, colorValue, colorValue);
        
        float xPos = static_cast<float>(x);
        float yStart = static_cast<float>(drawStart);
        float yEnd = static_cast<float>(drawEnd);
        
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yStart), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yEnd), wallColor));
        
        // ceiling
        if (drawStart > 0)
        {
            float lightSourceCeiling = std::max(0.0f, avgLighting - ambientComponent);
            float foggedAmbientCeiling = ambientComponent * distanceFog;
            float finalCeilingLight = lightSourceCeiling + foggedAmbientCeiling;
            
            int ceilingValue = static_cast<int>(finalCeilingLight * 20.0f);
            
            sf::Color ceilingColor(ceilingValue, ceilingValue, ceilingValue);
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, 0.0f), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, 0.0f), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yStart), ceilingColor));
        }
        
        // floor (slightly brighter than ceiling)
        if (drawEnd < m_screenHeight)
        {
            float lightSourceFloor = std::max(0.0f, avgLighting - ambientComponent);
            float foggedAmbientFloor = ambientComponent * distanceFog;
            float finalFloorLight = lightSourceFloor + foggedAmbientFloor;
            
            int floorValue = static_cast<int>(finalFloorLight * 30.0f);
            
            sf::Color floorColor(floorValue, floorValue, floorValue);
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yEnd), floorColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), floorColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, static_cast<float>(m_screenHeight)), floorColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, static_cast<float>(m_screenHeight)), floorColor));
        }
    }
    
    // floor/ceiling first, walls on top
    window.draw(m_floorCeiling);
    window.draw(m_wallSlices);
}
