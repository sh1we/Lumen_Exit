#include "GameManager.h"
#include "Map.h"
#include "Player.h"
#include "Raycaster.h"
#include "Minimap.h"
#include "HUD.h"
#include "LightSystem.h"
#include "PostProcessing.h"
#include <iostream>

GameManager::GameManager(const GameConfig& config)
    : m_config(config)
    , m_map(nullptr)
    , m_player(nullptr)
    , m_raycaster(nullptr)
    , m_minimap(nullptr)
    , m_hud(nullptr)
    , m_lightSystem(nullptr)
    , m_postProcessing(nullptr)
{
}

GameManager::~GameManager()
{
    cleanup();
}

void GameManager::createNewGame()
{
    cleanup();
    
    std::cout << "Creating new game..." << std::endl;
    
    m_map = new Map(51, 51);
    
    float spawnX, spawnY;
    m_map->getSpawnPosition(spawnX, spawnY);
    m_player = new Player(spawnX, spawnY, 0.0f);
    
    m_raycaster = new Raycaster(m_config.screenWidth, m_config.screenHeight);
    m_raycaster->setLightingQuality(m_config.lightingQuality);
    
    m_minimap = new Minimap(m_config.screenWidth, m_config.screenHeight);
    m_hud = new HUD(m_config.screenWidth, m_config.screenHeight);
    
    m_lightSystem = new LightSystem();
    m_lightSystem->addRoomLights(*m_map);
    
    m_postProcessing = new PostProcessing(m_config.screenWidth, m_config.screenHeight);
    
    std::cout << "Game created successfully!" << std::endl;
}

void GameManager::cleanup()
{
    delete m_map;
    delete m_player;
    delete m_raycaster;
    delete m_minimap;
    delete m_hud;
    delete m_lightSystem;
    delete m_postProcessing;
    
    m_map = nullptr;
    m_player = nullptr;
    m_raycaster = nullptr;
    m_minimap = nullptr;
    m_hud = nullptr;
    m_lightSystem = nullptr;
    m_postProcessing = nullptr;
}

void GameManager::updateSettings(const GameConfig& config)
{
    if (m_raycaster != nullptr)
    {
        m_raycaster->setLightingQuality(config.lightingQuality);
        std::cout << "Lighting quality updated" << std::endl;
    }
}
