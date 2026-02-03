#include "Menu.h"
#include "../utils/ResourceManager.h"
#include "../utils/AudioManager.h"
#include <iostream>

Menu::Menu(float width, float height)
    : m_selectedItemIndex(0)
    , m_width(width)
    , m_height(height)
    , m_inGameMode(false)
{
    m_font = ResourceManager::getInstance().getFont();
    std::cout << "Font loaded from ResourceManager!" << std::endl;

    rebuildMenu();
}

void Menu::setInGameMode(bool inGame)
{
    if (m_inGameMode != inGame)
    {
        m_inGameMode = inGame;
        rebuildMenu();
    }
}

void Menu::rebuildMenu()
{
    m_menuItems.clear();
    m_selectedItemIndex = 0;
    
    std::vector<std::string> items;
    
    if (m_inGameMode)
    {
        items = { "CONTINUE", "NEW GAME", "SETTINGS", "EXIT" };
    }
    else
    {
        items = { "START GAME", "SETTINGS", "EXIT" };
    }
    
    for (size_t i = 0; i < items.size(); ++i)
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(items[i]);
        text.setCharacterSize(50);
        text.setFillColor(i == 0 ? sf::Color(200, 200, 200) : sf::Color(80, 80, 80));
        
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(sf::Vector2f(m_width / 2.0f, m_height / 2.0f + static_cast<float>(i) * 80.0f));
        
        m_menuItems.push_back(text);
    }
}

void Menu::draw(sf::RenderWindow& window)
{
    sf::Text title;
    title.setFont(m_font);
    title.setString("Lumen_Exit()");
    title.setCharacterSize(80);
    title.setFillColor(sf::Color(255, 255, 255));
    title.setStyle(sf::Text::Bold);
    
    sf::FloatRect titleRect = title.getLocalBounds();
    title.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    title.setPosition(sf::Vector2f(window.getSize().x / 2.0f, 150.0f));
    
    window.draw(title);
    
    sf::Text subtitle;
    subtitle.setFont(m_font);
    subtitle.setString("\"In the void of uninitialized memory, light is your only pointer.\"");
    subtitle.setCharacterSize(18);
    subtitle.setFillColor(sf::Color(150, 150, 150));
    subtitle.setStyle(sf::Text::Italic);
    
    sf::FloatRect subtitleRect = subtitle.getLocalBounds();
    subtitle.setOrigin(subtitleRect.left + subtitleRect.width / 2.0f, subtitleRect.top + subtitleRect.height / 2.0f);
    subtitle.setPosition(sf::Vector2f(window.getSize().x / 2.0f, 220.0f));
    
    window.draw(subtitle);
    
    for (const auto& item : m_menuItems)
    {
        window.draw(item);
    }
}

void Menu::moveUp()
{
    if (m_selectedItemIndex > 0)
    {
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(80, 80, 80));
        m_selectedItemIndex--;
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(200, 200, 200));
        AudioManager::getInstance().playSound("scroll", 70.0f);
    }
}

void Menu::moveDown()
{
    if (m_selectedItemIndex < static_cast<int>(m_menuItems.size()) - 1)
    {
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(80, 80, 80));
        m_selectedItemIndex++;
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(200, 200, 200));
        AudioManager::getInstance().playSound("scroll", 70.0f);
    }
}

void Menu::handleMouseMove(const sf::Vector2i& mousePos)
{
    for (size_t i = 0; i < m_menuItems.size(); ++i)
    {
        sf::FloatRect bounds = m_menuItems[i].getGlobalBounds();
        
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            if (static_cast<int>(i) != m_selectedItemIndex)
            {
                m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(80, 80, 80));
                m_selectedItemIndex = static_cast<int>(i);
                m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(200, 200, 200));
                AudioManager::getInstance().playSound("scroll", 70.0f);
            }
            return;
        }
    }
}

bool Menu::handleMouseClick(const sf::Vector2i& mousePos)
{
    for (size_t i = 0; i < m_menuItems.size(); ++i)
    {
        sf::FloatRect bounds = m_menuItems[i].getGlobalBounds();
        
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            m_selectedItemIndex = static_cast<int>(i);
            AudioManager::getInstance().playSound("click", 80.0f);
            return true;
        }
    }
    
    return false;
}
