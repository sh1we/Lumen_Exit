#include "SettingsMenu.h"
#include "ResourceManager.h"
#include <iostream>
#include <sstream>
#include <iomanip>

SettingsMenu::SettingsMenu(float width, float height, GameConfig& config)
    : m_width(width)
    , m_height(height)
    , m_config(config)
    , m_selectedOption(0)
    , m_needsRestart(false)
{
    m_font = ResourceManager::getInstance().getFont();
    
    m_resolutions = {
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440}
    };
    
    m_fpsOptions = {30, 60, 120, 144, 240};
    
    findCurrentResolution();
    findCurrentFps();
}

void SettingsMenu::findCurrentResolution()
{
    m_currentResolutionIndex = 0;
    for (size_t i = 0; i < m_resolutions.size(); ++i)
    {
        if (m_resolutions[i].width == m_config.screenWidth &&
            m_resolutions[i].height == m_config.screenHeight)
        {
            m_currentResolutionIndex = static_cast<int>(i);
            break;
        }
    }
}

void SettingsMenu::findCurrentFps()
{
    m_currentFpsIndex = 1;  // default 60
    for (size_t i = 0; i < m_fpsOptions.size(); ++i)
    {
        if (m_fpsOptions[i] == m_config.targetFPS)
        {
            m_currentFpsIndex = static_cast<int>(i);
            break;
        }
    }
}

void SettingsMenu::draw(sf::RenderWindow& window)
{
    sf::Text title;
    title.setFont(m_font);
    title.setString("SETTINGS");
    title.setCharacterSize(70);
    title.setFillColor(sf::Color(255, 255, 255));
    title.setStyle(sf::Text::Bold);
    
    sf::FloatRect titleRect = title.getLocalBounds();
    title.setOrigin(titleRect.left + titleRect.width / 2.0f, titleRect.top + titleRect.height / 2.0f);
    title.setPosition(sf::Vector2f(m_width / 2.0f, 100.0f));
    window.draw(title);
    
    float yPos = 200.0f;
    
    // mouse sensitivity
    {
        std::ostringstream sensStr;
        sensStr << "Mouse Sensitivity: " << std::fixed << std::setprecision(3) << m_config.mouseSensitivity;
        
        sf::Text sensText;
        sensText.setFont(m_font);
        sensText.setString(sensStr.str());
        sensText.setCharacterSize(24);
        sensText.setFillColor(m_selectedOption == 0 ? sf::Color(255, 255, 100) : sf::Color(200, 200, 200));
        
        sf::FloatRect sensRect = sensText.getLocalBounds();
        sensText.setOrigin(sensRect.left + sensRect.width / 2.0f, sensRect.top + sensRect.height / 2.0f);
        sensText.setPosition(sf::Vector2f(m_width / 2.0f, yPos));
        window.draw(sensText);
        
        // slider bar
        float barWidth = 400.0f;
        float barHeight = 15.0f;
        float barX = m_width / 2.0f - barWidth / 2.0f;
        float barY = yPos + 30.0f;
        
        sf::RectangleShape barBg(sf::Vector2f(barWidth, barHeight));
        barBg.setPosition(barX, barY);
        barBg.setFillColor(sf::Color(50, 50, 50));
        barBg.setOutlineColor(m_selectedOption == 0 ? sf::Color(255, 255, 100) : sf::Color(150, 150, 150));
        barBg.setOutlineThickness(2.0f);
        window.draw(barBg);
        
        float minSens = 0.0005f;
        float maxSens = 0.005f;
        float sensPercent = (m_config.mouseSensitivity - minSens) / (maxSens - minSens);
        sensPercent = std::max(0.0f, std::min(1.0f, sensPercent));
        
        sf::RectangleShape barFill(sf::Vector2f(barWidth * sensPercent, barHeight));
        barFill.setPosition(barX, barY);
        barFill.setFillColor(sf::Color(100, 200, 100));
        window.draw(barFill);
        
        yPos += 80.0f;
    }
    
    // resolution
    {
        std::ostringstream resStr;
        resStr << "Resolution: " << m_resolutions[m_currentResolutionIndex].width 
               << "x" << m_resolutions[m_currentResolutionIndex].height;
        
        sf::Text resText;
        resText.setFont(m_font);
        resText.setString(resStr.str());
        resText.setCharacterSize(24);
        resText.setFillColor(m_selectedOption == 1 ? sf::Color(255, 255, 100) : sf::Color(200, 200, 200));
        
        sf::FloatRect resRect = resText.getLocalBounds();
        resText.setOrigin(resRect.left + resRect.width / 2.0f, resRect.top + resRect.height / 2.0f);
        resText.setPosition(sf::Vector2f(m_width / 2.0f, yPos));
        window.draw(resText);
        
        yPos += 50.0f;
    }
    
    // fps
    {
        std::ostringstream fpsStr;
        fpsStr << "Target FPS: " << m_fpsOptions[m_currentFpsIndex];
        
        sf::Text fpsText;
        fpsText.setFont(m_font);
        fpsText.setString(fpsStr.str());
        fpsText.setCharacterSize(24);
        fpsText.setFillColor(m_selectedOption == 2 ? sf::Color(255, 255, 100) : sf::Color(200, 200, 200));
        
        sf::FloatRect fpsRect = fpsText.getLocalBounds();
        fpsText.setOrigin(fpsRect.left + fpsRect.width / 2.0f, fpsRect.top + fpsRect.height / 2.0f);
        fpsText.setPosition(sf::Vector2f(m_width / 2.0f, yPos));
        window.draw(fpsText);
        
        yPos += 50.0f;
    }
    
    // fullscreen
    {
        std::ostringstream fullscreenStr;
        fullscreenStr << "Fullscreen: " << (m_config.fullscreen ? "ON" : "OFF");
        
        sf::Text fullscreenText;
        fullscreenText.setFont(m_font);
        fullscreenText.setString(fullscreenStr.str());
        fullscreenText.setCharacterSize(24);
        fullscreenText.setFillColor(m_selectedOption == 3 ? sf::Color(255, 255, 100) : sf::Color(200, 200, 200));
        
        sf::FloatRect fullscreenRect = fullscreenText.getLocalBounds();
        fullscreenText.setOrigin(fullscreenRect.left + fullscreenRect.width / 2.0f, fullscreenRect.top + fullscreenRect.height / 2.0f);
        fullscreenText.setPosition(sf::Vector2f(m_width / 2.0f, yPos));
        window.draw(fullscreenText);
        
        yPos += 80.0f;
    }
    
    // hints
    sf::Text hint;
    hint.setFont(m_font);
    hint.setString("UP/DOWN: Select | LEFT/RIGHT: Change | ESC: Back");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(150, 150, 150));
    
    sf::FloatRect hintRect = hint.getLocalBounds();
    hint.setOrigin(hintRect.left + hintRect.width / 2.0f, hintRect.top + hintRect.height / 2.0f);
    hint.setPosition(sf::Vector2f(m_width / 2.0f, m_height - 80.0f));
    window.draw(hint);
    
    // restart warning
    if (m_needsRestart)
    {
        sf::Text warning;
        warning.setFont(m_font);
        warning.setString("Restart required for changes to take effect");
        warning.setCharacterSize(20);
        warning.setFillColor(sf::Color(255, 100, 100));
        
        sf::FloatRect warnRect = warning.getLocalBounds();
        warning.setOrigin(warnRect.left + warnRect.width / 2.0f, warnRect.top + warnRect.height / 2.0f);
        warning.setPosition(sf::Vector2f(m_width / 2.0f, m_height - 50.0f));
        window.draw(warning);
    }
}

void SettingsMenu::handleInput(sf::Keyboard::Key key)
{
    if (key == sf::Keyboard::Up)
    {
        m_selectedOption = (m_selectedOption - 1 + 4) % 4;
    }
    else if (key == sf::Keyboard::Down)
    {
        m_selectedOption = (m_selectedOption + 1) % 4;
    }
    else if (key == sf::Keyboard::Left)
    {
        if (m_selectedOption == 0) // Sensitivity
        {
            m_config.mouseSensitivity -= 0.0002f;
            if (m_config.mouseSensitivity < 0.0005f)
                m_config.mouseSensitivity = 0.0005f;
            m_config.saveToFile("config.txt");
        }
        else if (m_selectedOption == 1) // Resolution
        {
            m_currentResolutionIndex = (m_currentResolutionIndex - 1 + static_cast<int>(m_resolutions.size())) % static_cast<int>(m_resolutions.size());
            m_config.screenWidth = m_resolutions[m_currentResolutionIndex].width;
            m_config.screenHeight = m_resolutions[m_currentResolutionIndex].height;
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
        else if (m_selectedOption == 2) // FPS
        {
            m_currentFpsIndex = (m_currentFpsIndex - 1 + static_cast<int>(m_fpsOptions.size())) % static_cast<int>(m_fpsOptions.size());
            m_config.targetFPS = m_fpsOptions[m_currentFpsIndex];
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
        else if (m_selectedOption == 3) // Fullscreen
        {
            m_config.fullscreen = !m_config.fullscreen;
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
    }
    else if (key == sf::Keyboard::Right)
    {
        if (m_selectedOption == 0) // Sensitivity
        {
            m_config.mouseSensitivity += 0.0002f;
            if (m_config.mouseSensitivity > 0.005f)
                m_config.mouseSensitivity = 0.005f;
            m_config.saveToFile("config.txt");
        }
        else if (m_selectedOption == 1) // Resolution
        {
            m_currentResolutionIndex = (m_currentResolutionIndex + 1) % static_cast<int>(m_resolutions.size());
            m_config.screenWidth = m_resolutions[m_currentResolutionIndex].width;
            m_config.screenHeight = m_resolutions[m_currentResolutionIndex].height;
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
        else if (m_selectedOption == 2) // FPS
        {
            m_currentFpsIndex = (m_currentFpsIndex + 1) % static_cast<int>(m_fpsOptions.size());
            m_config.targetFPS = m_fpsOptions[m_currentFpsIndex];
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
        else if (m_selectedOption == 3) // Fullscreen
        {
            m_config.fullscreen = !m_config.fullscreen;
            m_config.saveToFile("config.txt");
            m_needsRestart = true;
        }
    }
}

void SettingsMenu::handleMouseClick(const sf::Vector2i& mousePos)
{
    // TODO: click on options
}

void SettingsMenu::handleMouseMove(const sf::Vector2i& mousePos, bool isPressed)
{
    // TODO
}

void SettingsMenu::updateSensitivityFromMouse(float mouseX)
{
    // TODO
}
