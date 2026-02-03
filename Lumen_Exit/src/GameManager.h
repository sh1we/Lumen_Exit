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

// Менеджер игровых объектов - убирает дублирование кода
class GameManager
{
public:
    GameManager(const GameConfig& config);
    ~GameManager();
    
    // Запрещаем копирование
    GameManager(const GameManager&) = delete;
    GameManager& operator=(const GameManager&) = delete;
    
    // Создание новой игры
    void createNewGame();
    
    // Очистка всех объектов
    void cleanup();
    
    // Обновление настроек (например, качество освещения)
    void updateSettings(const GameConfig& config);
    
    // Геттеры
    Map* getMap() const { return m_map; }
    Player* getPlayer() const { return m_player; }
    Raycaster* getRaycaster() const { return m_raycaster; }
    Minimap* getMinimap() const { return m_minimap; }
    HUD* getHUD() const { return m_hud; }
    LightSystem* getLightSystem() const { return m_lightSystem; }
    PostProcessing* getPostProcessing() const { return m_postProcessing; }
    
    // Проверка инициализации
    bool isInitialized() const { return m_map != nullptr; }
    
private:
    const GameConfig& m_config;
    
    // Игровые объекты
    Map* m_map;
    Player* m_player;
    Raycaster* m_raycaster;
    Minimap* m_minimap;
    HUD* m_hud;
    LightSystem* m_lightSystem;
    PostProcessing* m_postProcessing;
};
