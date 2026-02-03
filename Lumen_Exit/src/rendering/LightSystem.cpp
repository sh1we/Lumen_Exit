#include "LightSystem.h"
#include "../world/Player.h"
#include "../world/Map.h"
#include "../utils/MathUtils.h"
#include <algorithm>
#include <cmath>
#include <immintrin.h>

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
    m_visibleLightIndices.clear();
    m_visibilityCache.clear();
}

void LightSystem::updateVisibleLights(const Player& player)
{
    m_visibleLightIndices.clear();
    m_visibilityCache.clear();  // clear cache each frame
    
    float playerAngle = player.getAngle();
    float playerX = player.getX();
    float playerY = player.getY();
    
    // FOV for culling (wider than actual FOV to catch edge lights)
    const float cullFOV = MathUtils::PI * 0.8f;  // ~144 degrees
    
    for (int i = 0; i < static_cast<int>(m_staticLights.size()); ++i)
    {
        const Light& light = m_staticLights[i];
        
        float dx = light.x - playerX;
        float dy = light.y - playerY;
        float distSq = dx * dx + dy * dy;
        
        // skip if too far
        float maxDist = light.radius + 20.0f;
        if (distSq > maxDist * maxDist)
            continue;
        
        // frustum check - is light in front of player?
        float angleToLight = std::atan2(dy, dx);
        float angleDiff = MathUtils::normalize_angle(angleToLight - playerAngle);
        
        // include if within extended FOV OR very close (might be behind but still visible on walls)
        float dist = std::sqrt(distSq);
        if (std::abs(angleDiff) < cullFOV || dist < light.radius)
        {
            m_visibleLightIndices.push_back(i);
        }
    }
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
    
    float dirX = dx / distance;
    float dirY = dy / distance;
    
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

bool LightSystem::hasLineOfSightCached(int lightIdx, float x2, float y2, const Map& map) const
{
    // just call regular version - cache disabled for thread safety with OpenMP
    const Light& light = m_staticLights[lightIdx];
    return hasLineOfSight(light.x, light.y, x2, y2, map);
}

float LightSystem::calculateLighting(float x, float y, const Player& player, const Map& map) const
{
    float totalLight = m_ambientLight;
    
    // use frustum-culled lights if available, otherwise check all
    const std::vector<int>& lightsToCheck = m_visibleLightIndices;
    bool useAllLights = lightsToCheck.empty();
    
    if (useAllLights)
    {
        // fallback - check all lights
        for (int idx = 0; idx < static_cast<int>(m_staticLights.size()); ++idx)
        {
            const Light& light = m_staticLights[idx];
            
            float dx = x - light.x;
            float dy = y - light.y;
            
            float distanceSq = dx * dx + dy * dy;
            float radiusSq = light.radius * light.radius;
            
            if (distanceSq < radiusSq)
            {
                if (hasLineOfSight(light.x, light.y, x, y, map))
                {
                    float distance = MathUtils::fast_sqrt(distanceSq);
                    float attenuation = 1.0f - (distance / light.radius);
                    attenuation = attenuation * attenuation;
                    
                    totalLight += light.intensity * attenuation;
                }
            }
        }
    }
    else
    {
        // use frustum-culled lights
        for (int idx : lightsToCheck)
        {
            const Light& light = m_staticLights[idx];
            
            float dx = x - light.x;
            float dy = y - light.y;
            
            float distanceSq = dx * dx + dy * dy;
            float radiusSq = light.radius * light.radius;
            
            if (distanceSq < radiusSq)
            {
                if (hasLineOfSightCached(idx, x, y, map))
                {
                    float distance = MathUtils::fast_sqrt(distanceSq);
                    float attenuation = 1.0f - (distance / light.radius);
                    attenuation = attenuation * attenuation;
                    
                    totalLight += light.intensity * attenuation;
                }
            }
        }
    }
    
    // flashlight
    if (m_flashlightEnabled && m_flashlightBattery > 0.0f)
    {
        float dx = x - player.getX();
        float dy = y - player.getY();
        float distanceSq = dx * dx + dy * dy;
        float radiusSq = m_flashlightRadius * m_flashlightRadius;
        
        if (distanceSq < radiusSq && distanceSq > 0.0001f)
        {
            float angleToPoint = std::atan2(dy, dx);
            float playerAngle = player.getAngle();
            float angleDiff = MathUtils::normalize_angle(angleToPoint - playerAngle);
            
            if (std::abs(angleDiff) < m_flashlightAngle)
            {
                if (hasLineOfSight(player.getX(), player.getY(), x, y, map))
                {
                    float distance = MathUtils::fast_sqrt(distanceSq);
                    
                    float distanceAttenuation = 1.0f - (distance / m_flashlightRadius);
                    distanceAttenuation = distanceAttenuation * distanceAttenuation * distanceAttenuation;
                    
                    float angleAttenuation = 1.0f - (std::abs(angleDiff) / m_flashlightAngle);
                    angleAttenuation = angleAttenuation * angleAttenuation;
                    
                    float batteryMultiplier = m_flashlightBattery / 100.0f;
                    if (m_flashlightBattery < 20.0f)
                    {
                        batteryMultiplier *= 0.5f + 0.5f * std::sin(m_flashlightBattery * 10.0f);
                    }
                    
                    totalLight += distanceAttenuation * angleAttenuation * batteryMultiplier * 2.5f;
                }
            }
        }
    }
    
    return MathUtils::clamp(totalLight, 0.0f, 1.0f);
}

// SIMD version - process 4 points at once
void LightSystem::calculateLighting4(const float* px, const float* py, float* results,
                                     const Player& player, const Map& map) const
{
#ifdef __AVX__
    __m128 totalLight = _mm_set1_ps(m_ambientLight);
    __m128 pointX = _mm_loadu_ps(px);
    __m128 pointY = _mm_loadu_ps(py);
    
    // process visible lights
    for (int idx : m_visibleLightIndices)
    {
        const Light& light = m_staticLights[idx];
        
        __m128 lightX = _mm_set1_ps(light.x);
        __m128 lightY = _mm_set1_ps(light.y);
        __m128 radius = _mm_set1_ps(light.radius);
        __m128 radiusSq = _mm_mul_ps(radius, radius);
        __m128 intensity = _mm_set1_ps(light.intensity);
        
        __m128 dx = _mm_sub_ps(pointX, lightX);
        __m128 dy = _mm_sub_ps(pointY, lightY);
        __m128 distSq = _mm_add_ps(_mm_mul_ps(dx, dx), _mm_mul_ps(dy, dy));
        
        // mask for points within radius
        __m128 inRange = _mm_cmplt_ps(distSq, radiusSq);
        
        // check if any point is in range
        int mask = _mm_movemask_ps(inRange);
        if (mask == 0) continue;
        
        // calculate attenuation for all 4 (will mask out later)
        __m128 dist = _mm_sqrt_ps(distSq);
        __m128 invRadius = _mm_div_ps(_mm_set1_ps(1.0f), radius);
        __m128 atten = _mm_sub_ps(_mm_set1_ps(1.0f), _mm_mul_ps(dist, invRadius));
        atten = _mm_mul_ps(atten, atten);  // squared falloff
        
        // check visibility for each point that's in range
        float attenArr[4];
        _mm_storeu_ps(attenArr, atten);
        
        for (int i = 0; i < 4; ++i)
        {
            if ((mask & (1 << i)) && hasLineOfSightCached(idx, px[i], py[i], map))
            {
                results[i] += light.intensity * attenArr[i];
            }
        }
    }
    
    // store base results
    _mm_storeu_ps(results, _mm_add_ps(_mm_loadu_ps(results), _mm_sub_ps(totalLight, _mm_set1_ps(m_ambientLight))));
    
    // flashlight - still scalar for now (complex angle math)
    if (m_flashlightEnabled && m_flashlightBattery > 0.0f)
    {
        for (int i = 0; i < 4; ++i)
        {
            float dx = px[i] - player.getX();
            float dy = py[i] - player.getY();
            float distanceSq = dx * dx + dy * dy;
            
            if (distanceSq < m_flashlightRadius * m_flashlightRadius && distanceSq > 0.0001f)
            {
                float angleToPoint = std::atan2(dy, dx);
                float angleDiff = MathUtils::normalize_angle(angleToPoint - player.getAngle());
                
                if (std::abs(angleDiff) < m_flashlightAngle)
                {
                    if (hasLineOfSight(player.getX(), player.getY(), px[i], py[i], map))
                    {
                        float distance = std::sqrt(distanceSq);
                        float distAtten = 1.0f - (distance / m_flashlightRadius);
                        distAtten = distAtten * distAtten * distAtten;
                        
                        float angleAtten = 1.0f - (std::abs(angleDiff) / m_flashlightAngle);
                        angleAtten = angleAtten * angleAtten;
                        
                        float battery = m_flashlightBattery / 100.0f;
                        if (m_flashlightBattery < 20.0f)
                            battery *= 0.5f + 0.5f * std::sin(m_flashlightBattery * 10.0f);
                        
                        results[i] += distAtten * angleAtten * battery * 2.5f;
                    }
                }
            }
        }
    }
    
    // clamp results
    for (int i = 0; i < 4; ++i)
    {
        results[i] = MathUtils::clamp(results[i], 0.0f, 1.0f);
    }
#else
    // fallback - just call scalar version
    for (int i = 0; i < 4; ++i)
    {
        results[i] = calculateLighting(px[i], py[i], player, map);
    }
#endif
}
