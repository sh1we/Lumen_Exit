#include "HUD.h"
#include "../world/Player.h"
#include "../rendering/LightSystem.h"
#include "../utils/ResourceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

HUD::HUD(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    m_font = ResourceManager::getInstance().getFont();
}

void HUD::draw(sf::RenderWindow& window, const Player& player, float gameTime, const LightSystem& lightSystem, unsigned int seed)
{
    sf::RectangleShape hudBackground(sf::Vector2f(280.0f, 140.0f));
    hudBackground.setPosition(10.0f, static_cast<float>(m_screenHeight) - 150.0f);
    hudBackground.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(hudBackground);
    
    float yPos = static_cast<float>(m_screenHeight) - 140.0f;
    
    // time
    int minutes = static_cast<int>(gameTime) / 60;
    int seconds = static_cast<int>(gameTime) % 60;
    std::ostringstream timeStr;
    timeStr << "Time: " << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << seconds;
    
    sf::Text timeText;
    timeText.setFont(m_font);
    timeText.setString(timeStr.str());
    timeText.setCharacterSize(16);
    timeText.setFillColor(sf::Color(200, 200, 200));
    timeText.setPosition(20.0f, yPos);
    window.draw(timeText);
    
    yPos += 25.0f;
    
    // battery
    float battery = lightSystem.getFlashlightBattery();
    std::ostringstream batteryStr;
    batteryStr << "Battery: " << std::fixed << std::setprecision(0) << battery << "%";
    
    sf::Text batteryText;
    batteryText.setFont(m_font);
    batteryText.setString(batteryStr.str());
    batteryText.setCharacterSize(16);
    
    if (battery > 50.0f)
        batteryText.setFillColor(sf::Color(100, 255, 100));
    else if (battery > 20.0f)
        batteryText.setFillColor(sf::Color(255, 255, 100));
    else
        batteryText.setFillColor(sf::Color(255, 100, 100));
    
    batteryText.setPosition(20.0f, yPos);
    window.draw(batteryText);
    
    if (!lightSystem.isFlashlightEnabled())
    {
        sf::Text flashlightOff;
        flashlightOff.setFont(m_font);
        flashlightOff.setString("[OFF]");
        flashlightOff.setCharacterSize(14);
        flashlightOff.setFillColor(sf::Color(255, 100, 100));
        flashlightOff.setPosition(180.0f, yPos);
        window.draw(flashlightOff);
    }
    else if (battery <= 0.0f)
    {
        sf::Text batteryDead;
        batteryDead.setFont(m_font);
        batteryDead.setString("[DEAD]");
        batteryDead.setCharacterSize(14);
        batteryDead.setFillColor(sf::Color(255, 50, 50));
        batteryDead.setPosition(180.0f, yPos);
        window.draw(batteryDead);
    }
    
    yPos += 20.0f;
    
    // battery bar
    float batteryBarWidth = 250.0f;
    float batteryBarHeight = 12.0f;
    float batteryPercent = battery / 100.0f;
    
    sf::RectangleShape batteryBarBg(sf::Vector2f(batteryBarWidth, batteryBarHeight));
    batteryBarBg.setPosition(20.0f, yPos);
    batteryBarBg.setFillColor(sf::Color(50, 50, 50));
    batteryBarBg.setOutlineColor(sf::Color(150, 150, 150));
    batteryBarBg.setOutlineThickness(1.0f);
    window.draw(batteryBarBg);
    
    sf::RectangleShape batteryBar(sf::Vector2f(batteryBarWidth * batteryPercent, batteryBarHeight));
    batteryBar.setPosition(20.0f, yPos);
    
    if (batteryPercent > 0.5f)
        batteryBar.setFillColor(sf::Color(100, 255, 100));
    else if (batteryPercent > 0.2f)
        batteryBar.setFillColor(sf::Color(255, 255, 100));
    else
        batteryBar.setFillColor(sf::Color(255, 100, 100));
    
    window.draw(batteryBar);
    
    yPos += 20.0f;
    
    // stamina
    std::ostringstream staminaStr;
    staminaStr << "Stamina: " << static_cast<int>(player.getStaminaPercent()) << "%";
    
    sf::Text staminaText;
    staminaText.setFont(m_font);
    staminaText.setString(staminaStr.str());
    staminaText.setCharacterSize(16);
    
    if (player.isStaminaExhausted())
        staminaText.setFillColor(sf::Color(255, 100, 100));
    else
        staminaText.setFillColor(sf::Color(200, 200, 200));
    
    staminaText.setPosition(20.0f, yPos);
    window.draw(staminaText);
    
    if (player.isStaminaExhausted())
    {
        sf::Text exhausted;
        exhausted.setFont(m_font);
        exhausted.setString("[EXHAUSTED]");
        exhausted.setCharacterSize(14);
        exhausted.setFillColor(sf::Color(255, 100, 100));
        exhausted.setPosition(180.0f, yPos);
        window.draw(exhausted);
    }
    
    yPos += 20.0f;
    
    // stamina bar
    float staminaBarWidth = 250.0f;
    float staminaBarHeight = 12.0f;
    float staminaPercent = player.getStamina() / player.getMaxStamina();
    
    sf::RectangleShape staminaBarBg(sf::Vector2f(staminaBarWidth, staminaBarHeight));
    staminaBarBg.setPosition(20.0f, yPos);
    staminaBarBg.setFillColor(sf::Color(50, 50, 50));
    staminaBarBg.setOutlineColor(sf::Color(150, 150, 150));
    staminaBarBg.setOutlineThickness(1.0f);
    window.draw(staminaBarBg);
    
    sf::RectangleShape staminaBar(sf::Vector2f(staminaBarWidth * staminaPercent, staminaBarHeight));
    staminaBar.setPosition(20.0f, yPos);
    
    if (player.isStaminaExhausted())
        staminaBar.setFillColor(sf::Color(255, 50, 50));
    else if (staminaPercent > 0.5f)
        staminaBar.setFillColor(sf::Color(100, 255, 100));
    else if (staminaPercent > 0.25f)
        staminaBar.setFillColor(sf::Color(255, 255, 100));
    else
        staminaBar.setFillColor(sf::Color(255, 100, 100));
    
    window.draw(staminaBar);
    
    // threshold marker when exhausted
    if (player.isStaminaExhausted())
    {
        float threshold = player.getExhaustionThreshold();
        sf::RectangleShape marker(sf::Vector2f(2.0f, staminaBarHeight + 4.0f));
        marker.setPosition(20.0f + staminaBarWidth * threshold - 1.0f, yPos - 2.0f);
        marker.setFillColor(sf::Color(255, 255, 255));
        window.draw(marker);
    }
    
    // controls hint
    sf::Text hintText;
    hintText.setFont(m_font);
    hintText.setString("TAB: Map | F: Flashlight");
    hintText.setCharacterSize(14);
    hintText.setFillColor(sf::Color(150, 150, 150));
    hintText.setPosition(static_cast<float>(m_screenWidth) - 280.0f, static_cast<float>(m_screenHeight) - 25.0f);
    window.draw(hintText);
    
    // seed display (top right)
    if (seed != 0)
    {
        sf::Text seedText;
        seedText.setFont(m_font);
        seedText.setString("Seed: " + std::to_string(seed));
        seedText.setCharacterSize(14);
        seedText.setFillColor(sf::Color(100, 100, 100));
        seedText.setPosition(static_cast<float>(m_screenWidth) - 200.0f, 10.0f);
        window.draw(seedText);
    }
}
