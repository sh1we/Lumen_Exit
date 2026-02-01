#include "HUD.h"
#include "Player.h"
#include <iostream>
#include <sstream>
#include <iomanip>

HUD::HUD(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
{
    if (!m_font.loadFromFile("C:\\Windows\\Fonts\\cour.ttf"))
    {
        std::cerr << "Error loading font for HUD!" << std::endl;
    }
}

void HUD::draw(sf::RenderWindow& window, const Player& player, float gameTime)
{
    // Полупрозрачный фон для HUD
    sf::RectangleShape hudBackground(sf::Vector2f(300.0f, 100.0f));
    hudBackground.setPosition(10.0f, static_cast<float>(m_screenHeight) - 110.0f);
    hudBackground.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(hudBackground);
    
    // Время
    int minutes = static_cast<int>(gameTime) / 60;
    int seconds = static_cast<int>(gameTime) % 60;
    std::ostringstream timeStr;
    timeStr << "Time: " << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << seconds;
    
    sf::Text timeText;
    timeText.setFont(m_font);
    timeText.setString(timeStr.str());
    timeText.setCharacterSize(18);
    timeText.setFillColor(sf::Color(200, 200, 200));
    timeText.setPosition(20.0f, static_cast<float>(m_screenHeight) - 100.0f);
    window.draw(timeText);
    
    // Стамина - текст
    std::ostringstream staminaStr;
    staminaStr << "Stamina: " << static_cast<int>(player.getStaminaPercent()) << "%";
    
    // Добавляем индикатор истощения
    if (player.isStaminaExhausted())
    {
        staminaStr << " [EXHAUSTED]";
    }
    
    sf::Text staminaText;
    staminaText.setFont(m_font);
    staminaText.setString(staminaStr.str());
    staminaText.setCharacterSize(18);
    
    // Красный текст если истощение
    if (player.isStaminaExhausted())
        staminaText.setFillColor(sf::Color(255, 100, 100));
    else
        staminaText.setFillColor(sf::Color(200, 200, 200));
    
    staminaText.setPosition(20.0f, static_cast<float>(m_screenHeight) - 75.0f);
    window.draw(staminaText);
    
    // Стамина - бар
    float barWidth = 200.0f;
    float barHeight = 15.0f;
    float staminaPercent = player.getStamina() / player.getMaxStamina();
    
    // Фон бара
    sf::RectangleShape staminaBarBg(sf::Vector2f(barWidth, barHeight));
    staminaBarBg.setPosition(20.0f, static_cast<float>(m_screenHeight) - 50.0f);
    staminaBarBg.setFillColor(sf::Color(50, 50, 50));
    staminaBarBg.setOutlineColor(sf::Color(150, 150, 150));
    staminaBarBg.setOutlineThickness(1.0f);
    window.draw(staminaBarBg);
    
    // Заполнение бара
    sf::RectangleShape staminaBar(sf::Vector2f(barWidth * staminaPercent, barHeight));
    staminaBar.setPosition(20.0f, static_cast<float>(m_screenHeight) - 50.0f);
    
    // Цвет зависит от уровня стамины
    if (player.isStaminaExhausted())
        staminaBar.setFillColor(sf::Color(255, 50, 50)); // Ярко-красный при истощении
    else if (staminaPercent > 0.5f)
        staminaBar.setFillColor(sf::Color(100, 255, 100)); // Зеленый
    else if (staminaPercent > 0.25f)
        staminaBar.setFillColor(sf::Color(255, 255, 100)); // Желтый
    else
        staminaBar.setFillColor(sf::Color(255, 100, 100)); // Красный
    
    window.draw(staminaBar);
    
    // Маркер порога восстановления (для визуализации)
    if (player.isStaminaExhausted())
    {
        float threshold = player.getExhaustionThreshold();
        sf::RectangleShape marker(sf::Vector2f(2.0f, barHeight + 4.0f));
        marker.setPosition(20.0f + barWidth * threshold - 1.0f, static_cast<float>(m_screenHeight) - 52.0f);
        marker.setFillColor(sf::Color(255, 255, 255));
        window.draw(marker);
        
        // Текст порога
        std::ostringstream thresholdStr;
        thresholdStr << static_cast<int>(threshold * 100) << "%";
        sf::Text thresholdText;
        thresholdText.setFont(m_font);
        thresholdText.setString(thresholdStr.str());
        thresholdText.setCharacterSize(12);
        thresholdText.setFillColor(sf::Color(255, 255, 255));
        thresholdText.setPosition(20.0f + barWidth * threshold + 5.0f, static_cast<float>(m_screenHeight) - 50.0f);
        window.draw(thresholdText);
    }
    
    // Подсказка Tab
    sf::Text hintText;
    hintText.setFont(m_font);
    hintText.setString("Press TAB for map");
    hintText.setCharacterSize(14);
    hintText.setFillColor(sf::Color(150, 150, 150));
    hintText.setPosition(static_cast<float>(m_screenWidth) - 200.0f, static_cast<float>(m_screenHeight) - 30.0f);
    window.draw(hintText);
}
