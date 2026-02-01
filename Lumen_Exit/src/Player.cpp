#include "Player.h"
#include "Map.h"
#include <SFML/Window/Keyboard.hpp>

Player::Player(float x, float y, float angle)
    : m_x(x), m_y(y), m_angle(angle)
    , m_moveSpeed(3.0f)
    , m_rotSpeed(2.5f)
    , m_sprint(false)
{
}

void Player::handleInput(float deltaTime)
{
    // Проверка Shift для ускорения
    m_sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift);
    float speed = m_sprint ? m_moveSpeed * 1.8f : m_moveSpeed;
    
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
    handleInput(deltaTime);
    
    // Проверка коллизий со стенами
    int tileX = static_cast<int>(m_x);
    int tileY = static_cast<int>(m_y);
    
    if (map.isWall(tileX, tileY))
    {
        // Если игрок в стене, отталкиваем его назад
        m_x -= getDirX() * 0.1f;
        m_y -= getDirY() * 0.1f;
    }
}
