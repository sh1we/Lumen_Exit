#include "PostProcessing.h"
#include <cmath>

PostProcessing::PostProcessing(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_vignetteStrength(0.6f)
    , m_headBobbingEnabled(true)
{
}

void PostProcessing::applyEffects(sf::RenderWindow& window, float walkBobbing, float batteryPercent)
{
    drawVignette(window, m_vignetteStrength);
    
    if (batteryPercent < 30.0f)
    {
        drawLowBatteryEffect(window, batteryPercent);
    }
}

void PostProcessing::drawVignette(sf::RenderWindow& window, float intensity)
{
    const int segments = 4;
    
    float centerX = static_cast<float>(m_screenWidth) / 2.0f;
    float centerY = static_cast<float>(m_screenHeight) / 2.0f;
    float maxRadius = std::sqrt(centerX * centerX + centerY * centerY);
    
    sf::VertexArray vignette(sf::Quads);
    
    // gradient quads for each edge
    {
        float alpha = intensity * 200.0f;
        sf::Color edgeColor(0, 0, 0, static_cast<sf::Uint8>(alpha));
        sf::Color centerColor(0, 0, 0, 0);
        
        // top
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), 0.0f), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) * 0.2f), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight) * 0.2f), centerColor));
        
        // bottom
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight) * 0.8f), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) * 0.8f), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight)), edgeColor));
        
        // left
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.2f, 0.0f), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.2f, static_cast<float>(m_screenHeight)), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight)), edgeColor));
        
        // right
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.8f, 0.0f), centerColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), 0.0f), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)), edgeColor));
        vignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.8f, static_cast<float>(m_screenHeight)), centerColor));
    }
    
    window.draw(vignette);
}

void PostProcessing::drawLowBatteryEffect(sf::RenderWindow& window, float batteryPercent)
{
    // red vignette when battery is dying
    float intensity = (30.0f - batteryPercent) / 30.0f;
    intensity = std::max(0.0f, std::min(1.0f, intensity));
    
    // pulsing effect
    float pulse = 0.5f + 0.5f * std::sin(batteryPercent * 0.5f);
    intensity *= pulse;
    
    sf::Uint8 alpha = static_cast<sf::Uint8>(intensity * 100.0f);
    
    sf::VertexArray redVignette(sf::Quads);
    sf::Color edgeColor(255, 0, 0, alpha);
    sf::Color centerColor(255, 0, 0, 0);
    
    // top
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), 0.0f), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) * 0.15f), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight) * 0.15f), centerColor));
    
    // bottom
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight) * 0.85f), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight) * 0.85f), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight)), edgeColor));
    
    // left
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, 0.0f), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.15f, 0.0f), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.15f, static_cast<float>(m_screenHeight)), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(0.0f, static_cast<float>(m_screenHeight)), edgeColor));
    
    // right
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.85f, 0.0f), centerColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), 0.0f), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth), static_cast<float>(m_screenHeight)), edgeColor));
    redVignette.append(sf::Vertex(sf::Vector2f(static_cast<float>(m_screenWidth) * 0.85f, static_cast<float>(m_screenHeight)), centerColor));
    
    window.draw(redVignette);
}
