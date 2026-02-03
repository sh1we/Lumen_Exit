#include "VictoryScreen.h"
#include "../utils/ResourceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

VictoryScreen::VictoryScreen(float width, float height, float completionTime, float bestTime, unsigned int seed)
    : m_timer(0.0f)
    , m_currentLine(0)
    , m_finished(false)
    , m_width(width)
    , m_height(height)
{
    m_font = ResourceManager::getInstance().getFont();
    
    int minutes = static_cast<int>(completionTime) / 60;
    int seconds = static_cast<int>(completionTime) % 60;
    
    std::ostringstream timeStr;
    timeStr << std::setfill('0') << std::setw(2) << minutes << ":" 
            << std::setfill('0') << std::setw(2) << seconds;
    
    std::string bestTimeStr;
    if (bestTime < 999999.0f)
    {
        int bestMinutes = static_cast<int>(bestTime) / 60;
        int bestSeconds = static_cast<int>(bestTime) % 60;
        std::ostringstream bestStr;
        bestStr << std::setfill('0') << std::setw(2) << bestMinutes << ":" 
                << std::setfill('0') << std::setw(2) << bestSeconds;
        bestTimeStr = bestStr.str();
    }
    else
    {
        bestTimeStr = "--:--";
    }
    
    bool isNewRecord = (completionTime < bestTime);
    
    m_lines.push_back("> call Exit()");
    m_lines.push_back("");
    m_lines.push_back("[ EXIT FOUND ]");
    m_lines.push_back("[ LIGHT HAS BEEN RESTORED ]");
    m_lines.push_back("");
    m_lines.push_back("=== STATISTICS ===");
    m_lines.push_back("Time: " + timeStr.str());
    m_lines.push_back("Best: " + bestTimeStr);
    m_lines.push_back("Seed: " + std::to_string(seed));
    if (isNewRecord && bestTime < 999999.0f)
    {
        m_lines.push_back("Status: NEW RECORD!");
    }
    else if (isNewRecord)
    {
        m_lines.push_back("Status: FIRST ESCAPE!");
    }
    else
    {
        m_lines.push_back("Status: ESCAPED");
    }
    m_lines.push_back("");
    m_lines.push_back("> Memory freed successfully.");
    m_lines.push_back("> Press any key to return to menu...");
}

void VictoryScreen::update(float deltaTime)
{
    if (m_finished)
        return;
    
    m_timer += deltaTime;
    
    if (m_timer > 0.3f && m_currentLine < static_cast<int>(m_lines.size()))
    {
        m_currentLine++;
        m_timer = 0.0f;
    }
    
    if (m_currentLine >= static_cast<int>(m_lines.size()))
    {
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
    float startY = m_height / 2.0f - 200.0f;
    
    for (int i = 0; i < m_currentLine && i < static_cast<int>(m_lines.size()); ++i)
    {
        sf::Text text;
        text.setFont(m_font);
        text.setString(m_lines[i]);
        text.setCharacterSize(24);
        text.setFillColor(sf::Color(0, 255, 0));
        text.setPosition(100.0f, startY + i * 35.0f);
        window.draw(text);
    }
    
    // blinking cursor
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
