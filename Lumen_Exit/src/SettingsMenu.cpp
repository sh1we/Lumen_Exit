#include "SettingsMenu.h"
#include <iostream>
#include <sstream>
#include <iomanip>

SettingsMenu::SettingsMenu(float width, float height, float& sensitivity)
    : m_width(width)
    , m_height(height)
    , m_sensitivity(sensitivity)
    , m_barWidth(400.0f)
    , m_barHeight(20.0f)
{
    if (!m_font.loadFromFile("C:\\Windows\\Fonts\\cour.ttf"))
    {
        std::cerr << "Error loading font for settings menu!" << std::endl;
    }
    
    // Вычисляем позицию слайдера
    m_barX = m_width / 2.0f - m_barWidth / 2.0f;
    m_barY = m_height / 2.0f + 60.0f;
}

void SettingsMenu::draw(sf::RenderWindow& window)
{
    // Заголовок
    sf::Text title;
    title.setFont(m_font);
    title.setString("SETTINGS");
    title.setCharacterSize(70);
    title.setFillColor(sf::Color(255, 255, 255));
    title.setStyle(sf::Text::Bold);
    
    sf::FloatRect titleRect = title.getLocalBounds();
    title.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    title.setPosition(sf::Vector2f(m_width / 2.0f, 150.0f));
    window.draw(title);
    
    // Чувствительность мыши
    std::ostringstream sensStr;
    sensStr << "Mouse Sensitivity: " << std::fixed << std::setprecision(3) << m_sensitivity;
    
    sf::Text sensText;
    sensText.setFont(m_font);
    sensText.setString(sensStr.str());
    sensText.setCharacterSize(30);
    sensText.setFillColor(sf::Color(200, 200, 200));
    
    sf::FloatRect sensRect = sensText.getLocalBounds();
    sensText.setOrigin(sensRect.left + sensRect.width / 2.0f, sensRect.top + sensRect.height / 2.0f);
    sensText.setPosition(sf::Vector2f(m_width / 2.0f, m_height / 2.0f - 50.0f));
    window.draw(sensText);
    
    // Подсказки управления
    sf::Text hint1;
    hint1.setFont(m_font);
    hint1.setString("Use LEFT/RIGHT arrows or click on the bar");
    hint1.setCharacterSize(20);
    hint1.setFillColor(sf::Color(150, 150, 150));
    
    sf::FloatRect hint1Rect = hint1.getLocalBounds();
    hint1.setOrigin(hint1Rect.left + hint1Rect.width / 2.0f, hint1Rect.top + hint1Rect.height / 2.0f);
    hint1.setPosition(sf::Vector2f(m_width / 2.0f, m_height / 2.0f + 20.0f));
    window.draw(hint1);
    
    // Визуальный бар чувствительности
    m_barX = m_width / 2.0f - m_barWidth / 2.0f;
    m_barY = m_height / 2.0f + 60.0f;
    
    // Фон бара
    sf::RectangleShape barBg(sf::Vector2f(m_barWidth, m_barHeight));
    barBg.setPosition(m_barX, m_barY);
    barBg.setFillColor(sf::Color(50, 50, 50));
    barBg.setOutlineColor(sf::Color(150, 150, 150));
    barBg.setOutlineThickness(2.0f);
    window.draw(barBg);
    
    // Заполнение бара (0.0005 - 0.005 диапазон)
    float minSens = 0.0005f;
    float maxSens = 0.005f;
    float sensPercent = (m_sensitivity - minSens) / (maxSens - minSens);
    sensPercent = std::max(0.0f, std::min(1.0f, sensPercent));
    
    sf::RectangleShape barFill(sf::Vector2f(m_barWidth * sensPercent, m_barHeight));
    barFill.setPosition(m_barX, m_barY);
    barFill.setFillColor(sf::Color(100, 200, 100));
    window.draw(barFill);
    
    // Подсказка выхода
    sf::Text hint2;
    hint2.setFont(m_font);
    hint2.setString("Press ESC to return to menu");
    hint2.setCharacterSize(20);
    hint2.setFillColor(sf::Color(150, 150, 150));
    
    sf::FloatRect hint2Rect = hint2.getLocalBounds();
    hint2.setOrigin(hint2Rect.left + hint2Rect.width / 2.0f, hint2Rect.top + hint2Rect.height / 2.0f);
    hint2.setPosition(sf::Vector2f(m_width / 2.0f, m_height - 100.0f));
    window.draw(hint2);
}

void SettingsMenu::handleInput(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Left)
    {
        m_sensitivity -= 0.0002f;
        if (m_sensitivity < 0.0005f)
            m_sensitivity = 0.0005f;
    }
    else if (key == sf::Keyboard::Right)
    {
        m_sensitivity += 0.0002f;
        if (m_sensitivity > 0.005f)
            m_sensitivity = 0.005f;
    }
}

void SettingsMenu::updateSensitivityFromMouse(float mouseX)
{
    // Вычисляем процент от позиции мыши на слайдере
    float percent = (mouseX - m_barX) / m_barWidth;
    percent = std::max(0.0f, std::min(1.0f, percent));
    
    // Конвертируем в значение чувствительности
    float minSens = 0.0005f;
    float maxSens = 0.005f;
    m_sensitivity = minSens + percent * (maxSens - minSens);
}

void SettingsMenu::handleMouseClick(const sf::Vector2i& mousePos)
{
    // Проверяем, кликнули ли на слайдер
    if (mousePos.x >= m_barX && mousePos.x <= m_barX + m_barWidth &&
        mousePos.y >= m_barY && mousePos.y <= m_barY + m_barHeight)
    {
        updateSensitivityFromMouse(static_cast<float>(mousePos.x));
    }
}

void SettingsMenu::handleMouseMove(const sf::Vector2i& mousePos, bool isPressed)
{
    // Если зажата кнопка мыши и курсор над слайдером, обновляем значение
    if (isPressed)
    {
        if (mousePos.x >= m_barX - 10 && mousePos.x <= m_barX + m_barWidth + 10 &&
            mousePos.y >= m_barY - 10 && mousePos.y <= m_barY + m_barHeight + 10)
        {
            updateSensitivityFromMouse(static_cast<float>(mousePos.x));
        }
    }
}
