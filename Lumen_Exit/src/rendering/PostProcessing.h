#pragma once
#include <SFML/Graphics.hpp>

class PostProcessing
{
public:
    PostProcessing(int screenWidth, int screenHeight);
    
    void applyEffects(sf::RenderWindow& window, float walkBobbing, float batteryPercent);
    
    void setVignetteStrength(float strength) { m_vignetteStrength = strength; }
    void setHeadBobbingEnabled(bool enabled) { m_headBobbingEnabled = enabled; }
    
private:
    int m_screenWidth;
    int m_screenHeight;
    
    float m_vignetteStrength;
    bool m_headBobbingEnabled;
    
    void drawVignette(sf::RenderWindow& window, float intensity);
    void drawLowBatteryEffect(sf::RenderWindow& window, float batteryPercent);
};
