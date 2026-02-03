#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// Утилиты для создания UI элементов - убирает дублирование кода
namespace UIHelper
{
    // Создание текста с автоматическим центрированием
    inline sf::Text createText(
        const sf::Font& font,
        const std::string& str,
        unsigned int size,
        const sf::Color& color,
        float x, float y,
        bool centered = true,
        sf::Text::Style style = sf::Text::Regular)
    {
        sf::Text text;
        text.setFont(font);
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
        text.setStyle(style);
        
        if (centered)
        {
            sf::FloatRect bounds = text.getLocalBounds();
            text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
        }
        
        text.setPosition(x, y);
        return text;
    }
    
    // Создание прямоугольника (для фонов, баров и т.д.)
    inline sf::RectangleShape createRect(
        float width, float height,
        float x, float y,
        const sf::Color& fillColor,
        const sf::Color& outlineColor = sf::Color::Transparent,
        float outlineThickness = 0.0f)
    {
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(fillColor);
        rect.setOutlineColor(outlineColor);
        rect.setOutlineThickness(outlineThickness);
        return rect;
    }
    
    // Создание прогресс-бара
    struct ProgressBar
    {
        sf::RectangleShape background;
        sf::RectangleShape fill;
        
        ProgressBar(float x, float y, float width, float height,
                   const sf::Color& bgColor, const sf::Color& fillColor)
        {
            background = createRect(width, height, x, y, bgColor);
            fill = createRect(width, height, x, y, fillColor);
        }
        
        void setValue(float percent)
        {
            percent = std::max(0.0f, std::min(100.0f, percent));
            float width = background.getSize().x * (percent / 100.0f);
            fill.setSize(sf::Vector2f(width, fill.getSize().y));
        }
        
        void draw(sf::RenderWindow& window)
        {
            window.draw(background);
            window.draw(fill);
        }
    };
    
    // Конвертация float в цвет (0.0-1.0 -> 0-255)
    inline sf::Color floatToColor(float r, float g, float b, float a = 1.0f)
    {
        return sf::Color(
            static_cast<sf::Uint8>(r * 255.0f),
            static_cast<sf::Uint8>(g * 255.0f),
            static_cast<sf::Uint8>(b * 255.0f),
            static_cast<sf::Uint8>(a * 255.0f)
        );
    }
    
    // Интерполяция цветов
    inline sf::Color lerpColor(const sf::Color& a, const sf::Color& b, float t)
    {
        t = std::max(0.0f, std::min(1.0f, t));
        return sf::Color(
            static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
            static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
            static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
            static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
        );
    }
    
    // Clamp цвета в диапазон 0-255
    inline int clampColor(int value)
    {
        return std::max(0, std::min(255, value));
    }
    
    // Создание цвета из яркости (grayscale)
    inline sf::Color brightnessToColor(float brightness)
    {
        int value = clampColor(static_cast<int>(brightness * 255.0f));
        return sf::Color(value, value, value);
    }
}
