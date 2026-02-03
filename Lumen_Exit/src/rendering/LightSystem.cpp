#include "LightSystem.h"
#include "../world/Player.h"
#include "../world/Map.h"
#include "../utils/MathUtils.h"
#include <algorithm>
#include <cmath>

LightSystem::LightSystem()
    : m_flashlightEnabled(true)
    , m_flashlightBattery(100.0f)
    , m_flashlightRadius(12.0f)
    , m_flashlightAngle(1.2f)
    , m_flashlightDrainRate(3.0f)
    , m_ambientLight(0.03f)
{
}

void LightSystem::addRoomLights(const Map& map)
{
    clearLights();
    
    const std::vector<Room>& rooms = map.getRooms();
    
    for (const auto& room : rooms)
    {
        float centerX = room.x + room.width / 2.0f;
        float centerY = room.y + room.height / 2.0f;
        
        float radius = std::max(room.width, room.height) * 1.5f;
        
        sf::Color lightColor = room.isExit ? sf::Color(255, 215, 100) : sf::Color(255, 240, 200);
        
        m_staticLights.emplace_back(centerX, centerY, radius, 2.0f, lightColor, true);
    }
}

void LightSystem::clearLights()
{
    m_staticLights.clear();
}

void LightSystem::updateFlashlight(float deltaTime, bool isUsing, bool inSafeRoom)
{
    if (inSafeRoom && m_flashlightBattery < 100.0f)
    {
        m_flashlightBattery += 20.0f * deltaTime;
        
        if (m_flashlightBattery > 100.0f)
        {
            m_flashlightBattery = 100.0f;
        }
    }
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
    float dx = x2 - x1;
    float dy = y2 - y1;
    
    float distance = MathUtils::fast_distance(dx, dy);
    
    if (distance < 0.1f)
        return true;
    
    // normalize direction
    float dirX = dx / distance;
    float dirY = dy / distance;
    
    // check slightly before the target point to avoid hitting the wall we're checking
    float checkDist = distance - 0.1f;
    if (checkDist < 0.1f)
        return true;
    
    int steps = static_cast<int>(checkDist) + 1;
    
    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / static_cast<float>(steps);
        float checkX = x1 + dirX * checkDist * t;
        float checkY = y1 + dirY * checkDist * t;
        
        if (map.isWall(static_cast<int>(checkX), static_cast<int>(checkY)))
        {
            return false;
        }
    }
    
    return true;
}

float LightSystem::calculateLighting(float x, float y, const Player& player, const Map& map) const
{
    float totalLight = m_ambientLight;
    
    for (const auto& light : m_staticLights)
    {
        float dx = x - light.x;
        float dy = y - light.y;
        
        float distanceSq = MathUtils::distance_squared(dx, dy);
        float radiusSq = light.radius * light.radius;
        
        if (distanceSq < radiusSq)
        {
            float distance = MathUtils::fast_sqrt(distanceSq);
            
            if (hasLineOfSight(light.x, light.y, x, y, map))
            {
                float attenuation = 1.0f - (distance / light.radius);
                attenuation = attenuation * attenuation;
                
                float lightContribution = light.intensity * attenuation;
                totalLight += lightContribution;
            }
        }
    }
    
    if (m_flashlightEnabled && m_flashlightBattery > 0.0f)
    {
        float dx = x - player.getX();
        float dy = y - player.getY();
        float distanceSq = MathUtils::distance_squared(dx, dy);
        float radiusSq = m_flashlightRadius * m_flashlightRadius;
        
        if (distanceSq < radiusSq && distanceSq > 0.0001f)
        {
            float distance = MathUtils::fast_sqrt(distanceSq);
            
            float angleToPoint = std::atan2(dy, dx);
            float playerAngle = player.getAngle();
            
            float angleDiff = MathUtils::normalize_angle(angleToPoint - playerAngle);
            
            if (std::abs(angleDiff) < m_flashlightAngle)
            {
                if (hasLineOfSight(player.getX(), player.getY(), x, y, map))
                {
                    float distanceAttenuation = 1.0f - (distance / m_flashlightRadius);
                    distanceAttenuation = distanceAttenuation * distanceAttenuation * distanceAttenuation;
                    
                    float angleAttenuation = 1.0f - (std::abs(angleDiff) / m_flashlightAngle);
                    angleAttenuation = angleAttenuation * angleAttenuation;
                    
                    float batteryMultiplier = m_flashlightBattery / 100.0f;
                    if (m_flashlightBattery < 20.0f)
                    {
                        batteryMultiplier *= 0.5f + 0.5f * std::sin(m_flashlightBattery * 10.0f);
                    }
                    
                    float flashlightContribution = distanceAttenuation * angleAttenuation * batteryMultiplier * 2.5f;
                    totalLight += flashlightContribution;
                }
            }
        }
    }
    
    return MathUtils::clamp(totalLight, 0.0f, 1.0f);
}
