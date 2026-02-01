#include "Menu.h"
#include <iostream>

Menu::Menu(float width, float height)
    : m_selectedItemIndex(0)
    , m_width(width)
    , m_height(height)
    , m_inGameMode(false)
{
    // Используем моноширинный шрифт Courier New для ретро-вайба
    if (!m_font.loadFromFile("C:\\Windows\\Fonts\\cour.ttf"))
    {
        std::cerr << "Error loading Courier New font!" << std::endl;
    }
    else
    {
        std::cout << "Courier New font loaded successfully!" << std::endl;
    }

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
    
    // Создаем пункты меню в зависимости от режима
    std::vector<std::string> items;
    
    if (m_inGameMode)
    {
        // Меню во время игры
        items = { "CONTINUE", "NEW GAME", "SETTINGS", "EXIT" };
    }
    else
    {
        // Меню до начала игры
        items = { "START GAME", "SETTINGS", "EXIT" };
    }
    
    for (size_t i = 0; i < items.size(); ++i)
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(items[i]);
        text.setCharacterSize(50);
        text.setFillColor(i == 0 ? sf::Color(200, 200, 200) : sf::Color(80, 80, 80));
        
        // Центрируем текст
        sf::FloatRect textRect = text.getLocalBounds();
        text.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        text.setPosition(sf::Vector2f(m_width / 2.0f, m_height / 2.0f + static_cast<float>(i) * 80.0f));
        
        m_menuItems.push_back(text);
    }
}

void Menu::draw(sf::RenderWindow& window)
{
    // Заголовок игры
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
    
    // Подзаголовок
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
    
    // Пункты меню
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
    }
}

void Menu::moveDown()
{
    if (m_selectedItemIndex < static_cast<int>(m_menuItems.size()) - 1)
    {
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(80, 80, 80));
        m_selectedItemIndex++;
        m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(200, 200, 200));
    }
}

void Menu::handleMouseMove(const sf::Vector2i& mousePos)
{
    // Проверяем, наведена ли мышь на какой-то пункт меню
    for (size_t i = 0; i < m_menuItems.size(); ++i)
    {
        sf::FloatRect bounds = m_menuItems[i].getGlobalBounds();
        
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            // Если это не выбранный пункт, меняем выделение
            if (static_cast<int>(i) != m_selectedItemIndex)
            {
                m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(80, 80, 80));
                m_selectedItemIndex = static_cast<int>(i);
                m_menuItems[m_selectedItemIndex].setFillColor(sf::Color(200, 200, 200));
            }
            return;
        }
    }
}

bool Menu::handleMouseClick(const sf::Vector2i& mousePos)
{
    // Проверяем, кликнули ли на какой-то пункт меню
    for (size_t i = 0; i < m_menuItems.size(); ++i)
    {
        sf::FloatRect bounds = m_menuItems[i].getGlobalBounds();
        
        if (bounds.contains(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
        {
            m_selectedItemIndex = static_cast<int>(i);
            return true; // Клик был на пункте меню
        }
    }
    
    return false; // Клик был вне пунктов меню
}
