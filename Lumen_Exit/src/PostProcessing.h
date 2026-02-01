#pragma once
#include <SFML/Graphics.hpp>

class PostProcessing
{
public:
    PostProcessing(int screenWidth, int screenHeight);
    
    // Применяет эффекты к окну
    void applyEffects(sf::RenderWindow& window, float walkBobbing, float batteryPercent);
    
    // Параметры эффектов
    void setVignetteStrength(float strength) { m_vignetteStrength = strength; }
    void setHeadBobbingEnabled(bool enabled) { m_headBobbingEnabled = enabled; }
    
private:
    int m_screenWidth;
    int m_screenHeight;
    
    // Параметры эффектов
    float m_vignetteStrength;
    bool m_headBobbingEnabled;
    
    // Создает vignette эффект (затемнение по краям)
    void drawVignette(sf::RenderWindow& window, float intensity);
    
    // Создает эффект низкого заряда (красная виньетка)
    void drawLowBatteryEffect(sf::RenderWindow& window, float batteryPercent);
};
