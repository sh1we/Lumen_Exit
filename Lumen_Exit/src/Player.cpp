#include "Player.h"
#include "Map.h"
#include "MathUtils.h"
#include <SFML/Window/Keyboard.hpp>
#include <cmath>

Player::Player(float x, float y, float angle)
    : m_x(x), m_y(y), m_angle(angle)
    , m_moveSpeed(3.0f)
    , m_rotSpeed(2.5f)
    , m_sprint(false)
    , m_reachedExit(false)
    , m_stamina(100.0f)
    , m_maxStamina(100.0f)
    , m_staminaDrainRate(20.0f)
    , m_staminaRegenRate(8.0f)
    , m_staminaRegenDelay(1.5f)
    , m_staminaRegenTimer(0.0f)
    , m_staminaExhausted(false)
    , m_exhaustionThreshold(0.5f)
    , m_exhaustionIncrement(0.1f)      // +10% each time you exhaust
    , m_exhaustionResetTimer(0.0f)
    , m_exhaustionResetDelay(30.0f)    // resets after 30s of not sprinting
    , m_hadExhaustion(false)
{
    m_visitedTiles.clear();
    updateDirection();
}

void Player::updateDirection()
{
    MathUtils::sincos_fast(m_angle, m_cachedDirY, m_cachedDirX);
}

void Player::handleInput(float deltaTime)
{
    // exhaustion threshold reset timer (only ticks when not sprinting)
    if (!m_sprint && !m_staminaExhausted && m_exhaustionThreshold > 0.5f)
    {
        m_exhaustionResetTimer += deltaTime;
        
        if (m_exhaustionResetTimer >= m_exhaustionResetDelay)
        {
            m_exhaustionThreshold = 0.5f;
            m_exhaustionResetTimer = 0.0f;
            m_hadExhaustion = false;
        }
    }
    
    // can sprint if: stamina > 0 AND (not exhausted OR recovered past threshold)
    bool canSprint = m_stamina > 0.0f && (!m_staminaExhausted || m_stamina >= m_maxStamina * m_exhaustionThreshold);
    
    m_sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && canSprint;
    
    float speed = m_sprint ? m_moveSpeed * 1.3f : m_moveSpeed;
    
    if (m_sprint)
    {
        m_stamina -= m_staminaDrainRate * deltaTime;
        m_staminaRegenTimer = 0.0f;
        
        if (m_stamina <= 0.0f)
        {
            m_stamina = 0.0f;
            m_sprint = false;
            
            // progressive exhaustion - threshold goes up each time (punishes spam)
            if (m_hadExhaustion && m_exhaustionResetTimer < m_exhaustionResetDelay)
            {
                m_exhaustionThreshold += m_exhaustionIncrement;
                if (m_exhaustionThreshold > 1.0f)
                {
                    m_exhaustionThreshold = 1.0f;
                }
            }
            
            m_staminaExhausted = true;
            m_hadExhaustion = true;
            m_exhaustionResetTimer = 0.0f;
        }
    }
    else
    {
        m_staminaRegenTimer += deltaTime;
        
        if (m_staminaRegenTimer >= m_staminaRegenDelay)
        {
            m_stamina += m_staminaRegenRate * deltaTime;
            
            if (m_stamina >= m_maxStamina)
            {
                m_stamina = m_maxStamina;
            }
            
            if (m_staminaExhausted && m_stamina >= m_maxStamina * m_exhaustionThreshold)
            {
                m_staminaExhausted = false;
            }
        }
    }
    
    // WASD movement
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
    
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        m_angle -= m_rotSpeed * deltaTime;
        updateDirection();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        m_angle += m_rotSpeed * deltaTime;
        updateDirection();
    }
}

void Player::handleMouseMovement(float deltaX, float sensitivity)
{
    m_angle += deltaX * sensitivity;
    updateDirection();
}

void Player::update(float deltaTime, const Map& map)
{
    float oldX = m_x;
    float oldY = m_y;
    
    handleInput(deltaTime);
    
    // collision with small radius so player doesn't feel fat
    const float playerRadius = 0.15f;
    
    auto checkCollision = [&](float x, float y) -> bool {
        return map.isWall(static_cast<int>(x + playerRadius), static_cast<int>(y + playerRadius)) ||
               map.isWall(static_cast<int>(x - playerRadius), static_cast<int>(y + playerRadius)) ||
               map.isWall(static_cast<int>(x + playerRadius), static_cast<int>(y - playerRadius)) ||
               map.isWall(static_cast<int>(x - playerRadius), static_cast<int>(y - playerRadius));
    };
    
    bool fullCollision = checkCollision(m_x, m_y);
    
    if (fullCollision)
    {
        // wall sliding - try each axis separately
        bool collisionX = checkCollision(m_x, oldY);
        bool collisionY = checkCollision(oldX, m_y);
        
        if (collisionX && collisionY)
        {
            // stuck in corner, just revert
            m_x = oldX;
            m_y = oldY;
        }
        else if (collisionX)
        {
            m_x = oldX;  // slide along Y
        }
        else if (collisionY)
        {
            m_y = oldY;  // slide along X
        }
    }
    
    // fog of war - mark current tile as visited
    int tileX = static_cast<int>(m_x);
    int tileY = static_cast<int>(m_y);
    m_visitedTiles.insert(tileY * 1000 + tileX);
    
    // if in a room, reveal the whole room
    const std::vector<Room>& rooms = map.getRooms();
    for (const auto& room : rooms)
    {
        if (tileX >= room.x && tileX < room.x + room.width &&
            tileY >= room.y && tileY < room.y + room.height)
        {
            for (int ry = room.y; ry < room.y + room.height; ++ry)
            {
                for (int rx = room.x; rx < room.x + room.width; ++rx)
                {
                    m_visitedTiles.insert(ry * 1000 + rx);
                }
            }
            
            if (room.isExit && !m_reachedExit)
            {
                m_reachedExit = true;
            }
            
            break;
        }
    }
}

bool Player::isInRoom(const Map& map) const
{
    int tileX = static_cast<int>(m_x);
    int tileY = static_cast<int>(m_y);
    
    const std::vector<Room>& rooms = map.getRooms();
    for (const auto& room : rooms)
    {
        if (tileX >= room.x && tileX < room.x + room.width &&
            tileY >= room.y && tileY < room.y + room.height)
        {
            return true;
        }
    }
    
    return false;
}
