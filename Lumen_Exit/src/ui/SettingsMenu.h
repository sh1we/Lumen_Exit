#pragma once
#include <SFML/Graphics.hpp>
#include "../core/Config.h"

class SettingsMenu
{
public:
    SettingsMenu(float width, float height, GameConfig& config);
    
    void draw(sf::RenderWindow& window);
    void handleInput(sf::Keyboard::Key key);
    void handleMouseClick(const sf::Vector2i& mousePos);
    void handleMouseMove(const sf::Vector2i& mousePos, bool isPressed);
    void handleTextInput(sf::Uint32 unicode);
    
    bool needsRestart() const { return m_needsRestart; }
    void clearRestartFlag() { m_needsRestart = false; }
    bool isEditingSeed() const { return m_editingSeed; }
    
private:
    sf::Font m_font;
    float m_width;
    float m_height;
    GameConfig& m_config;
    
    int m_selectedOption;  // 0=sensitivity, 1=resolution, 2=fps, 3=fullscreen, 4=seed
    bool m_needsRestart;
    
    struct Resolution { int width; int height; };
    std::vector<Resolution> m_resolutions;
    int m_currentResolutionIndex;
    
    std::vector<int> m_fpsOptions;
    int m_currentFpsIndex;
    
    // seed input
    bool m_editingSeed;
    std::string m_seedInput;
    
    void updateSensitivityFromMouse(float mouseX);
    void findCurrentResolution();
    void findCurrentFps();
};
