#include "Map.h"
#include <stack>
#include <algorithm>
#include <iostream>

Map::Map(int width, int height, unsigned int seed)
    : m_width(width), m_height(height)
    , m_spawnX(1), m_spawnY(1)
{
    // Размеры должны быть нечетными для алгоритма
    if (m_width % 2 == 0) m_width++;
    if (m_height % 2 == 0) m_height++;
    
    m_tiles.resize(m_width * m_height, 1); // Заполняем стенами
    
    // Генерируем лабиринт
    if (seed == 0)
    {
        seed = static_cast<unsigned int>(std::time(nullptr));
    }
    
    std::cout << "Generating maze with seed: " << seed << std::endl;
    generateMaze(seed);
}

int Map::getTile(int x, int y) const
{
    if (x < 0 || x >= m_width || y < 0 || y >= m_height)
        return 1; // За границами карты - стена
    
    return m_tiles[y * m_width + x];
}

bool Map::isWall(int x, int y) const
{
    return getTile(x, y) == 1;
}

bool Map::isInRoom(int x, int y) const
{
    for (const auto& room : m_rooms)
    {
        if (x >= room.x && x < room.x + room.width &&
            y >= room.y && y < room.y + room.height)
        {
            return true;
        }
    }
    return false;
}

void Map::getSpawnPosition(float& outX, float& outY) const
{
    outX = static_cast<float>(m_spawnX) + 0.5f;
    outY = static_cast<float>(m_spawnY) + 0.5f;
}

bool Map::isValidCell(int x, int y) const
{
    return x > 0 && x < m_width - 1 && y > 0 && y < m_height - 1;
}

void Map::generateMaze(unsigned int seed)
{
    std::mt19937 rng(seed);
    
    // Сначала создаем комнаты (safe зоны)
    addRooms(rng, 6); // 6 случайных комнат
    
    // Начинаем лабиринт с позиции (1, 1)
    recursiveBacktracker(1, 1, rng);
    
    // Спавним игрока в центре первой комнаты
    if (!m_rooms.empty())
    {
        m_spawnX = m_rooms[0].centerX();
        m_spawnY = m_rooms[0].centerY();
    }
    else
    {
        m_spawnX = 1;
        m_spawnY = 1;
    }
    
    std::cout << "Maze generated! " << m_rooms.size() << " safe rooms created." << std::endl;
    std::cout << "Spawn at (" << m_spawnX << ", " << m_spawnY << ")" << std::endl;
}

void Map::recursiveBacktracker(int startX, int startY, std::mt19937& rng)
{
    std::stack<std::pair<int, int>> stack;
    stack.push({startX, startY});
    
    m_tiles[startY * m_width + startX] = 0; // Делаем стартовую клетку проходимой
    
    // Направления: вверх, вправо, вниз, влево
    const int dx[] = {0, 2, 0, -2};
    const int dy[] = {-2, 0, 2, 0};
    
    while (!stack.empty())
    {
        auto [x, y] = stack.top();
        
        // Собираем непосещенных соседей
        std::vector<int> neighbors;
        
        for (int i = 0; i < 4; ++i)
        {
            int nx = x + dx[i];
            int ny = y + dy[i];
            
            if (isValidCell(nx, ny) && m_tiles[ny * m_width + nx] == 1)
            {
                neighbors.push_back(i);
            }
        }
        
        if (neighbors.empty())
        {
            stack.pop();
        }
        else
        {
            // Выбираем случайного соседа
            std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size()) - 1);
            int dir = neighbors[dist(rng)];
            
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            
            // Убираем стену между текущей клеткой и соседом
            int wallX = x + dx[dir] / 2;
            int wallY = y + dy[dir] / 2;
            
            m_tiles[wallY * m_width + wallX] = 0;
            m_tiles[ny * m_width + nx] = 0;
            
            stack.push({nx, ny});
        }
    }
}

void Map::addRooms(std::mt19937& rng, int roomCount)
{
    std::uniform_int_distribution<int> sizeDistX(4, 4); // Фиксированный размер 4x4
    std::uniform_int_distribution<int> sizeDistY(4, 4);
    
    int attempts = 0;
    int maxAttempts = roomCount * 10; // Ограничиваем попытки
    
    while (m_rooms.size() < static_cast<size_t>(roomCount) && attempts < maxAttempts)
    {
        attempts++;
        
        int roomWidth = sizeDistX(rng);
        int roomHeight = sizeDistY(rng);
        
        // Случайная позиция для комнаты
        std::uniform_int_distribution<int> posDistX(3, m_width - roomWidth - 3);
        std::uniform_int_distribution<int> posDistY(3, m_height - roomHeight - 3);
        
        int roomX = posDistX(rng);
        int roomY = posDistY(rng);
        
        // Проверяем пересечение с существующими комнатами
        bool overlaps = false;
        for (const auto& existingRoom : m_rooms)
        {
            // Добавляем отступ минимум 3 клетки между комнатами
            if (!(roomX + roomWidth + 3 < existingRoom.x ||
                  roomX > existingRoom.x + existingRoom.width + 3 ||
                  roomY + roomHeight + 3 < existingRoom.y ||
                  roomY > existingRoom.y + existingRoom.height + 3))
            {
                overlaps = true;
                break;
            }
        }
        
        if (overlaps)
            continue;
        
        // Сохраняем информацию о комнате
        Room room;
        room.x = roomX;
        room.y = roomY;
        room.width = roomWidth;
        room.height = roomHeight;
        m_rooms.push_back(room);
        
        // Создаем комнату (убираем стены)
        for (int y = roomY; y < roomY + roomHeight; ++y)
        {
            for (int x = roomX; x < roomX + roomWidth; ++x)
            {
                if (isValidCell(x, y))
                {
                    m_tiles[y * m_width + x] = 0;
                }
            }
        }
    }
}
