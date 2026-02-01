#pragma once
#include <SFML/Graphics.hpp>
#include "Config.h"

class SettingsMenu
{
public:
    SettingsMenu(float width, float height, GameConfig& config);
    
    void draw(sf::RenderWindow& window);
    void handleInput(sf::Keyboard::Key key);
    void handleMouseClick(const sf::Vector2i& mousePos);
    void handleMouseMove(const sf::Vector2i& mousePos, bool isPressed);
    
    bool needsRestart() const { return m_needsRestart; }
    void clearRestartFlag() { m_needsRestart = false; }
    
private:
    sf::Font m_font;
    float m_width;
    float m_height;
    GameConfig& m_config;
    
    int m_selectedOption; // 0=sensitivity, 1=resolution, 2=fps, 3=fullscreen
    bool m_needsRestart;
    
    // Доступные разрешения
    struct Resolution { int width; int height; };
    std::vector<Resolution> m_resolutions;
    int m_currentResolutionIndex;
    
    // Доступные FPS
    std::vector<int> m_fpsOptions;
    int m_currentFpsIndex;
    
    void updateSensitivityFromMouse(float mouseX);
    void findCurrentResolution();
    void findCurrentFps();
};
