#include "Map.h"
#include <stack>
#include <algorithm>
#include <iostream>

Map::Map(int width, int height, unsigned int seed)
    : m_width(width), m_height(height)
    , m_spawnX(1), m_spawnY(1)
{
    // maze algo needs odd dimensions
    if (m_width % 2 == 0) m_width++;
    if (m_height % 2 == 0) m_height++;
    
    m_tiles.resize(m_width * m_height, 1);  // all walls
    
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
        return 1;  // out of bounds = wall
    
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

bool Map::isInExitRoom(int x, int y) const
{
    for (const auto& room : m_rooms)
    {
        if (room.isExit && 
            x >= room.x && x < room.x + room.width &&
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
    
    addRooms(rng, 6);
    recursiveBacktracker(1, 1, rng);
    
    // spawn in first room, exit in farthest room
    if (!m_rooms.empty())
    {
        m_spawnX = m_rooms[0].centerX();
        m_spawnY = m_rooms[0].centerY();
        
        if (m_rooms.size() > 1)
        {
            int maxDist = 0;
            int exitRoomIndex = static_cast<int>(m_rooms.size()) - 1;
            
            for (int i = 1; i < static_cast<int>(m_rooms.size()); ++i)
            {
                int dx = m_rooms[i].centerX() - m_spawnX;
                int dy = m_rooms[i].centerY() - m_spawnY;
                int dist = dx * dx + dy * dy;
                
                if (dist > maxDist)
                {
                    maxDist = dist;
                    exitRoomIndex = i;
                }
            }
            
            m_rooms[exitRoomIndex].isExit = true;
            std::cout << "Exit room at (" << m_rooms[exitRoomIndex].centerX() 
                      << ", " << m_rooms[exitRoomIndex].centerY() << ")" << std::endl;
        }
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
    
    m_tiles[startY * m_width + startX] = 0;
    
    // up, right, down, left
    const int dx[] = {0, 2, 0, -2};
    const int dy[] = {-2, 0, 2, 0};
    
    while (!stack.empty())
    {
        auto [x, y] = stack.top();
        
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
            std::uniform_int_distribution<int> dist(0, static_cast<int>(neighbors.size()) - 1);
            int dir = neighbors[dist(rng)];
            
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            
            // carve through the wall between cells
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
    std::uniform_int_distribution<int> sizeDistX(4, 4);  // fixed 4x4 rooms
    std::uniform_int_distribution<int> sizeDistY(4, 4);
    
    int attempts = 0;
    int maxAttempts = roomCount * 10;
    
    while (m_rooms.size() < static_cast<size_t>(roomCount) && attempts < maxAttempts)
    {
        attempts++;
        
        int roomWidth = sizeDistX(rng);
        int roomHeight = sizeDistY(rng);
        
        std::uniform_int_distribution<int> posDistX(3, m_width - roomWidth - 3);
        std::uniform_int_distribution<int> posDistY(3, m_height - roomHeight - 3);
        
        int roomX = posDistX(rng);
        int roomY = posDistY(rng);
        
        // check overlap with existing rooms (need 3 tile gap minimum)
        bool overlaps = false;
        for (const auto& existingRoom : m_rooms)
        {
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
        
        Room room;
        room.x = roomX;
        room.y = roomY;
        room.width = roomWidth;
        room.height = roomHeight;
        m_rooms.push_back(room);
        
        // carve out the room
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
