#pragma once
#include <cmath>
#include <set>

class Map;

class Player
{
public:
    Player(float x, float y, float angle);
    
    void update(float deltaTime, const Map& map);
    void handleInput(float deltaTime);
    
    float getX() const { return m_x; }
    float getY() const { return m_y; }
    float getAngle() const { return m_angle; }
    float getDirX() const { return std::cos(m_angle); }
    float getDirY() const { return std::sin(m_angle); }
    
    bool hasVisited(int x, int y) const { return m_visitedTiles.count(y * 1000 + x) > 0; }
    bool hasReachedExit() const { return m_reachedExit; }
    void setReachedExit(bool reached) { m_reachedExit = reached; }
    
    float getStamina() const { return m_stamina; }
    float getMaxStamina() const { return m_maxStamina; }
    float getStaminaPercent() const { return (m_stamina / m_maxStamina) * 100.0f; }
    bool isStaminaExhausted() const { return m_staminaExhausted; }
    
private:
    float m_x;          // Позиция X
    float m_y;          // Позиция Y
    float m_angle;      // Угол поворота (в радианах)
    
    float m_moveSpeed;  // Скорость движения
    float m_rotSpeed;   // Скорость поворота
    bool m_sprint;      // Ускорение
    
    std::set<int> m_visitedTiles; // Fog of war - посещенные клетки
    bool m_reachedExit; // Достиг ли игрок выхода
    
    // Система стамины
    float m_stamina;
    float m_maxStamina;
    float m_staminaDrainRate;
    float m_staminaRegenRate;
    float m_staminaRegenDelay;   // Задержка перед восстановлением
    float m_staminaRegenTimer;   // Таймер для задержки
    bool m_staminaExhausted;     // Флаг истощения (нужно восстановить до 50%)
};
