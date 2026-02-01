#include "Player.h"
#include "Map.h"
#include <SFML/Window/Keyboard.hpp>

Player::Player(float x, float y, float angle)
    : m_x(x), m_y(y), m_angle(angle)
    , m_moveSpeed(3.0f)
    , m_rotSpeed(2.5f)
    , m_sprint(false)
    , m_reachedExit(false)
{
    // Инициализируем fog of war (все клетки не посещены)
    m_visitedTiles.clear();
}

void Player::handleInput(float deltaTime)
{
    // Проверка Shift для ускорения
    m_sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    float speed = m_sprint ? m_moveSpeed * 1.3f : m_moveSpeed; // Уменьшил с 1.8 до 1.3
    
    // Движение вперед/назад (W/S)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        float newX = m_x + getDirX() * speed * deltaTime;
        float newY = m_y + getDirY() * speed * deltaTime;
        m_x = newX;
        m_y = newY;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        float newX = m_x - getDirX() * speed * deltaTime;
        float newY = m_y - getDirY() * speed * deltaTime;
        m_x = newX;
        m_y = newY;
    }
    
    // Поворот камеры (A/D)
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        m_angle -= m_rotSpeed * deltaTime;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        m_angle += m_rotSpeed * deltaTime;
    }
}

void Player::update(float deltaTime, const Map& map)
{
    // Сохраняем старую позицию
    float oldX = m_x;
    float oldY = m_y;
    
    handleInput(deltaTime);
    
    // Проверка коллизий со стенами с маленьким радиусом игрока
    const float playerRadius = 0.15f; // Уменьшенный радиус для узких коридоров
    
    // Проверяем 4 угла вокруг игрока
    bool collision = false;
    
    if (map.isWall(static_cast<int>(m_x + playerRadius), static_cast<int>(m_y + playerRadius)) ||
        map.isWall(static_cast<int>(m_x - playerRadius), static_cast<int>(m_y + playerRadius)) ||
        map.isWall(static_cast<int>(m_x + playerRadius), static_cast<int>(m_y - playerRadius)) ||
        map.isWall(static_cast<int>(m_x - playerRadius), static_cast<int>(m_y - playerRadius)))
    {
        collision = true;
    }
    
    // Если столкновение, возвращаем старую позицию
    if (collision)
    {
        m_x = oldX;
        m_y = oldY;
    }
    
    // Отмечаем текущую клетку как посещенную
    int tileX = static_cast<int>(m_x);
    int tileY = static_cast<int>(m_y);
    m_visitedTiles.insert(tileY * 1000 + tileX);
    
    // Если игрок в комнате, отмечаем всю комнату как посещенную
    const std::vector<Room>& rooms = map.getRooms();
    for (const auto& room : rooms)
    {
        if (tileX >= room.x && tileX < room.x + room.width &&
            tileY >= room.y && tileY < room.y + room.height)
        {
            // Отмечаем все клетки комнаты
            for (int ry = room.y; ry < room.y + room.height; ++ry)
            {
                for (int rx = room.x; rx < room.x + room.width; ++rx)
                {
                    m_visitedTiles.insert(ry * 1000 + rx);
                }
            }
            
            // Проверяем, достиг ли игрок выхода
            if (room.isExit && !m_reachedExit)
            {
                m_reachedExit = true;
            }
            
            break;
        }
    }
}
