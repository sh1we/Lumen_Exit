#include "VictoryScreen.h"
#include <iostream>
#include <sstream>
#include <iomanip>

VictoryScreen::VictoryScreen(float width, float height, float completionTime)
    : m_timer(0.0f)
    , m_currentLine(0)
    , m_finished(false)
    , m_width(width)
    , m_height(height)
{
    if (!m_font.loadFromFile("C:\\Windows\\Fonts\\cour.ttf"))
    {
        std::cerr << "Error loading font for victory screen!" << std::endl;
    }
    
    // Форматируем время
    int minutes = static_cast<int>(completionTime) / 60;
    int seconds = static_cast<int>(completionTime) % 60;
    
    std::ostringstream timeStr;
    timeStr << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << seconds;
    
    // Текст победы в стиле терминала
    m_lines.push_back("> call Exit()");
    m_lines.push_back("");
    m_lines.push_back("[ EXIT FOUND ]");
    m_lines.push_back("[ LIGHT HAS BEEN RESTORED ]");
    m_lines.push_back("");
    m_lines.push_back("=== STATISTICS ===");
    m_lines.push_back("Time: " + timeStr.str());
    m_lines.push_back("Status: ESCAPED");
    m_lines.push_back("");
    m_lines.push_back("> Memory freed successfully.");
    m_lines.push_back("> Press any key to return to menu...");
}

void VictoryScreen::update(float deltaTime)
{
    if (m_finished)
        return;
    
    m_timer += deltaTime;
    
    // Показываем строки постепенно
    if (m_timer > 0.3f && m_currentLine < static_cast<int>(m_lines.size()))
    {
        m_currentLine++;
        m_timer = 0.0f;
    }
    
    // Когда все строки показаны, ждем нажатия клавиши
    if (m_currentLine >= static_cast<int>(m_lines.size()))
    {
        // Проверяем нажатие любой клавиши
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Enter) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
        {
            m_finished = true;
        }
    }
}

void VictoryScreen::draw(sf::RenderWindow& window)
{
    // Отрисовка строк терминала
    float startY = m_height / 2.0f - 200.0f;
    
    for (int i = 0; i < m_currentLine && i < static_cast<int>(m_lines.size()); ++i)
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(m_lines[i]);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color(0, 255, 0)); // Зеленый терминальный цвет
        text.setPosition(100.0f, startY + i * 35.0f);
        window.draw(text);
    }
    
    // Мигающий курсор на последней строке
    if (m_currentLine > 0 && m_currentLine < static_cast<int>(m_lines.size()))
    {
        if (static_cast<int>(m_timer * 3.0f) % 2 == 0)
        {
            sf::Text cursor;
            cursor.setFont(m_font);
            cursor.setString("_");
            cursor.setCharacterSize(24);
            cursor.setFillColor(sf::Color(0, 255, 0));
            
            sf::Text lastLine;
            lastLine.setFont(m_font);
            lastLine.setString(m_lines[m_currentLine - 1]);
            lastLine.setCharacterSize(24);
            float textWidth = lastLine.getLocalBounds().width;
            
            cursor.setPosition(100.0f + textWidth, startY + (m_currentLine - 1) * 35.0f);
            window.draw(cursor);
        }
    }
}
