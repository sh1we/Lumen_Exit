#pragma once
#include <cmath>
#include <unordered_set>

class Map;

class Player
{
public:
    Player(float x, float y, float angle);
    
    void update(float deltaTime, const Map& map);
    void handleInput(float deltaTime);
    void handleMouseMovement(float deltaX, float sensitivity);
    
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getAngle() const { return m_angle; }
    float getDirX() const { return m_cachedDirX; } // ОПТИМИЗАЦИЯ: используем кэш
    float getDirY() const { return m_cachedDirY; } // ОПТИМИЗАЦИЯ: используем кэш
    
    bool hasVisited(int x, int y) const { return m_visitedTiles.count(y * 1000 + x) > 0; }
    bool hasReachedExit() const { return m_reachedExit; }
    void setReachedExit(bool reached) { m_reachedExit = reached; }
    
    // Проверка нахождения в комнате
    bool isInRoom(const Map& map) const;
    
    float getStamina() const { return m_stamina; }
    float getMaxStamina() const { return m_maxStamina; }
    float getStaminaPercent() const { return (m_stamina / m_maxStamina) * 100.0f; }
    bool isStaminaExhausted() const { return m_staminaExhausted; }
    float getExhaustionThreshold() const { return m_exhaustionThreshold; }
    
private:
    void updateDirection(); // ОПТИМИЗАЦИЯ: обновляет кэш направления
    
    float m_x;          // Позиция X
    float m_y;          // Позиция Y
    float m_angle;      // Угол поворота (в радианах)
    
    // ОПТИМИЗАЦИЯ: кэшированное направление
    float m_cachedDirX;
    float m_cachedDirY;
    
    float m_moveSpeed;  // Скорость движения
    float m_rotSpeed;   // Скорость поворота
    bool m_sprint;      // Ускорение
    
    std::unordered_set<int> m_visitedTiles; // Fog of war - посещенные клетки (оптимизировано)
    bool m_reachedExit; // Достиг ли игрок выхода
    
    // Система стамины
    float m_stamina;
    float m_maxStamina;
    float m_staminaDrainRate;
    float m_staminaRegenRate;
    float m_staminaRegenDelay;      // Задержка перед восстановлением
    float m_staminaRegenTimer;      // Таймер для задержки
    bool m_staminaExhausted;        // Флаг истощения
    float m_exhaustionThreshold;    // Порог восстановления (50%, 60%, 70%...)
    float m_exhaustionIncrement;    // На сколько увеличивается порог
    float m_exhaustionResetTimer;   // Таймер для сброса порога
    float m_exhaustionResetDelay;   // Время до сброса порога (30 сек)
    bool m_hadExhaustion;           // Было ли хотя бы одно истощение
};
