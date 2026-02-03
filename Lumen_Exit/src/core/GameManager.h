#pragma once
#include "Config.h"

// Forward declarations
class Map;
class Player;
class Raycaster;
class Minimap;
class HUD;
class LightSystem;
class PostProcessing;

class GameManager
{
public:
    GameManager(const GameConfig& config);
    ~GameManager();
    
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    void createNewGame();
    void cleanup();
    void updateSettings(const GameConfig& config);
    
    Map* getMap() const { return m_map; }
    Player* getPlayer() const { return m_player; }
    Raycaster* getRaycaster() const { return m_raycaster; }
    Minimap* getMinimap() const { return m_minimap; }
    HUD* getHUD() const { return m_hud; }
    LightSystem* getLightSystem() const { return m_lightSystem; }
    PostProcessing* getPostProcessing() const { return m_postProcessing; }
    
    bool isInitialized() const { return m_map != nullptr; }
    
private:
    const GameConfig& m_config;
    
    Map* m_map;
    Player* m_player;
    Raycaster* m_raycaster;
    Minimap* m_minimap;
    HUD* m_hud;
    LightSystem* m_lightSystem;
    PostProcessing* m_postProcessing;
};
