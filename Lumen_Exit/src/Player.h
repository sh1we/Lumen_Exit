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
    
private:
    float m_x;          // Позиция X
    float m_y;          // Позиция Y
    float m_angle;      // Угол поворота (в радианах)
    
    float m_moveSpeed;  // Скорость движения
    float m_rotSpeed;   // Скорость поворота
    bool m_sprint;      // Ускорение
    
    std::set<int> m_visitedTiles; // Fog of war - посещенные клетки
    bool m_reachedExit; // Достиг ли игрок выхода
};
