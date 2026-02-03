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
    m_lightingBuffer.resize(screenWidth, 0.0f);
    m_smoothedBuffer.resize(screenWidth, 0.0f);
    m_rayDataBuffer.resize(screenWidth);
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
    
    float fogDistance = lightSystem.isFlashlightEnabled() && lightSystem.getFlashlightBattery() > 0.0f ? 6.0f : 2.5f;
    float ambientComponent = 0.08f;  // 8% base visibility
    
    // PASS 1: calculate all ray data and lighting values
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
                useInterpolation = (x % 2 == 1);
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
        
        // calculate fog
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
        
        // calculate final brightness WITH side shading applied
        float lightSourceComponent = std::max(0.0f, avgLighting - ambientComponent);
        float foggedAmbient = ambientComponent * distanceFog;
        float finalLighting = lightSourceComponent + foggedAmbient;
        
        // apply side shading BEFORE storing - so smoothing blends the edges
        float sideFactor = hit.hitVertical ? 1.0f : 0.94f;
        float wallBrightness = finalLighting * sideFactor;
        
        // store raw data for pass 2
        m_rayDataBuffer[x].correctedDistance = correctedDistance;
        m_rayDataBuffer[x].drawStart = drawStart;
        m_rayDataBuffer[x].drawEnd = drawEnd;
        m_rayDataBuffer[x].hitVertical = hit.hitVertical;
        m_rayDataBuffer[x].rawLighting = avgLighting;
        m_rayDataBuffer[x].distanceFog = distanceFog;
        m_lightingBuffer[x] = wallBrightness;  // store with side shading already applied
    }
    
    // smooth the lighting buffer - 5-tap weighted filter
    // blends the harsh edge between vertical/horizontal walls
    for (int x = 0; x < m_screenWidth; ++x)
    {
        float sum = m_lightingBuffer[x] * 2.0f;  // center has more weight
        float weight = 2.0f;
        
        if (x > 0)
        {
            sum += m_lightingBuffer[x - 1];
            weight += 1.0f;
        }
        if (x > 1)
        {
            sum += m_lightingBuffer[x - 2] * 0.5f;
            weight += 0.5f;
        }
        if (x < m_screenWidth - 1)
        {
            sum += m_lightingBuffer[x + 1];
            weight += 1.0f;
        }
        if (x < m_screenWidth - 2)
        {
            sum += m_lightingBuffer[x + 2] * 0.5f;
            weight += 0.5f;
        }
        
        m_smoothedBuffer[x] = sum / weight;
    }
    
    // PASS 2: render using smoothed lighting
    for (int x = 0; x < m_screenWidth; ++x)
    {
        const RayData& data = m_rayDataBuffer[x];
        float wallBrightness = m_smoothedBuffer[x];
        
        int colorValue = static_cast<int>(wallBrightness * 255.0f);
        colorValue = std::max(0, std::min(255, colorValue));
        sf::Color wallColor(colorValue, colorValue, colorValue);
        
        float xPos = static_cast<float>(x);
        float yStart = static_cast<float>(data.drawStart);
        float yEnd = static_cast<float>(data.drawEnd);
        
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yStart), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yEnd), wallColor));
        m_wallSlices.append(sf::Vertex(sf::Vector2f(xPos, yEnd), wallColor));
        
        // ceiling
        if (data.drawStart > 0)
        {
            float lightSourceCeiling = std::max(0.0f, data.rawLighting - ambientComponent);
            float foggedAmbientCeiling = ambientComponent * data.distanceFog;
            float finalCeilingLight = lightSourceCeiling + foggedAmbientCeiling;
            
            int ceilingValue = static_cast<int>(finalCeilingLight * 20.0f);
            
            sf::Color ceilingColor(ceilingValue, ceilingValue, ceilingValue);
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, 0.0f), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, 0.0f), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos + 1.0f, yStart), ceilingColor));
            m_floorCeiling.append(sf::Vertex(sf::Vector2f(xPos, yStart), ceilingColor));
        }
        
        // floor (slightly brighter than ceiling)
        if (data.drawEnd < m_screenHeight)
        {
            float lightSourceFloor = std::max(0.0f, data.rawLighting - ambientComponent);
            float foggedAmbientFloor = ambientComponent * data.distanceFog;
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
