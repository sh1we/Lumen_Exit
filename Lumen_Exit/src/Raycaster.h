#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Map;
class Player;
class LightSystem;
enum class LightingQuality;

class Raycaster
{
public:
    Raycaster(int screenWidth, int screenHeight);
    
    void render(sf::RenderWindow& window, const Player& player, const Map& map, const LightSystem& lightSystem);
    
    void setLightingQuality(LightingQuality quality) { m_lightingQuality = quality; }
    
private:
    struct RayHit
    {
        float distance;
        bool hitVertical;
        int mapX;
        int mapY;
        float hitX;
        float hitY;
    };
    
    RayHit castRay(float rayAngle, const Player& player, const Map& map);
    
    int m_screenWidth;
    int m_screenHeight;
    float m_fov;
    
    sf::VertexArray m_floorCeiling;
    sf::VertexArray m_wallSlices;
    
    LightingQuality m_lightingQuality;
    
    float m_lastLighting;
    
    // smoothing buffer for lighting
    std::vector<float> m_lightingBuffer;
    std::vector<float> m_smoothedBuffer;
    
    // cached ray data for two-pass rendering
    struct RayData
    {
        float correctedDistance;
        int drawStart;
        int drawEnd;
        bool hitVertical;
        float rawLighting;
        float distanceFog;
    };
    std::vector<RayData> m_rayDataBuffer;
};
