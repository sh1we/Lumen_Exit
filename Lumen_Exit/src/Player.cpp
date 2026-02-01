#include "Player.h"
#include "Map.h"
#include <SFML/Window/Keyboard.hpp>

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
    , m_exhaustionThreshold(0.5f)      // Начальный порог 50%
    , m_exhaustionIncrement(0.1f)      // Увеличиваем на 10% каждый раз
    , m_exhaustionResetTimer(0.0f)
    , m_exhaustionResetDelay(30.0f)    // Сброс через 30 секунд
    , m_hadExhaustion(false)           // Изначально не было истощений
{
    // Инициализируем fog of war (все клетки не посещены)
    m_visitedTiles.clear();
}

void Player::handleInput(float deltaTime)
{
    // Обновляем таймер сброса порога истощения (только если не бежим и не истощены)
    if (!m_sprint && !m_staminaExhausted && m_exhaustionThreshold > 0.5f)
    {
        m_exhaustionResetTimer += deltaTime;
        
        // Если прошло 30 секунд без истощения, сбрасываем порог до 50%
        if (m_exhaustionResetTimer >= m_exhaustionResetDelay)
        {
            m_exhaustionThreshold = 0.5f;
            m_exhaustionResetTimer = 0.0f;
            m_hadExhaustion = false; // Сбрасываем флаг истощения
        }
    }
    
    // Проверка Shift для ускорения
    // Можно бежать только если:
    // 1. Стамина > 0
    // 2. Не в состоянии истощения (или стамина восстановилась до порога)
    bool canSprint = m_stamina > 0.0f && (!m_staminaExhausted || m_stamina >= m_maxStamina * m_exhaustionThreshold);
    
    m_sprint = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && canSprint;
    
    float speed = m_sprint ? m_moveSpeed * 1.3f : m_moveSpeed;
    
    // Управление стаминой
    if (m_sprint)
    {
        // Тратим стамину при беге
        m_stamina -= m_staminaDrainRate * deltaTime;
        m_staminaRegenTimer = 0.0f; // Сбрасываем таймер восстановления
        
        if (m_stamina <= 0.0f)
        {
            m_stamina = 0.0f;
            m_sprint = false;
            
            // Увеличиваем порог только если уже было истощение раньше
            // И прошло меньше 30 секунд с момента последнего восстановления
            if (m_hadExhaustion && m_exhaustionResetTimer < m_exhaustionResetDelay)
            {
                m_exhaustionThreshold += m_exhaustionIncrement;
                if (m_exhaustionThreshold > 1.0f)
                {
                    m_exhaustionThreshold = 1.0f; // Максимум 100%
                }
            }
            
            m_staminaExhausted = true;
            m_hadExhaustion = true; // Отмечаем что было истощение
            m_exhaustionResetTimer = 0.0f; // Сбрасываем таймер 30 секунд
        }
    }
    else
    {
        // Восстановление стамины с задержкой
        m_staminaRegenTimer += deltaTime;
        
        if (m_staminaRegenTimer >= m_staminaRegenDelay)
        {
            m_stamina += m_staminaRegenRate * deltaTime;
            
            if (m_stamina >= m_maxStamina)
            {
                m_stamina = m_maxStamina;
            }
            
            // Выходим из состояния истощения когда стамина >= порога
            if (m_staminaExhausted && m_stamina >= m_maxStamina * m_exhaustionThreshold)
            {
                m_staminaExhausted = false;
                // НЕ сбрасываем m_exhaustionResetTimer - он продолжает считать 30 секунд
            }
        }
    }
    
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

void Player::handleMouseMovement(float deltaX, float sensitivity)
{
    m_angle += deltaX * sensitivity;
}

void Player::update(float deltaTime, const Map& map)
{
    // Сохраняем старую позицию
    float oldX = m_x;
    float oldY = m_y;
    
    handleInput(deltaTime);
    
    // Проверка коллизий - используем маленький радиус
    const float playerRadius = 0.15f;
    
    // Проверяем все 4 угла игрока + центр для более точной коллизии
    auto checkCollision = [&](float x, float y) -> bool {
        // Проверяем 4 угла вокруг позиции
        return map.isWall(static_cast<int>(x + playerRadius), static_cast<int>(y + playerRadius)) ||
               map.isWall(static_cast<int>(x - playerRadius), static_cast<int>(y + playerRadius)) ||
               map.isWall(static_cast<int>(x + playerRadius), static_cast<int>(y - playerRadius)) ||
               map.isWall(static_cast<int>(x - playerRadius), static_cast<int>(y - playerRadius));
    };
    
    // Проверяем новую позицию
    bool fullCollision = checkCollision(m_x, m_y);
    
    if (fullCollision)
    {
        // Пробуем скользить только по X
        bool collisionX = checkCollision(m_x, oldY);
        
        // Пробуем скользить только по Y
        bool collisionY = checkCollision(oldX, m_y);
        
        if (collisionX && collisionY)
        {
            // Коллизия с обеих сторон - возвращаемся на старую позицию
            m_x = oldX;
            m_y = oldY;
        }
        else if (collisionX)
        {
            // Коллизия по X - скользим по Y
            m_x = oldX;
        }
        else if (collisionY)
        {
            // Коллизия по Y - скользим по X
            m_y = oldY;
        }
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
