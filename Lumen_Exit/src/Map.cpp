#include "Map.h"

Map::Map(int width, int height)
    : m_width(width), m_height(height)
{
    m_tiles.resize(width * height, 0);
    generateTestMap();
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

void Map::generateTestMap()
{
    // Простая тестовая карта 16x16
    // 1 = стена, 0 = пустота
    
    // Заполняем границы стенами
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            if (x == 0 || x == m_width - 1 || y == 0 || y == m_height - 1)
            {
                m_tiles[y * m_width + x] = 1;
            }
        }
    }
    
    // Добавляем несколько стен внутри для теста
    m_tiles[5 * m_width + 5] = 1;
    m_tiles[5 * m_width + 6] = 1;
    m_tiles[5 * m_width + 7] = 1;
    
    m_tiles[10 * m_width + 10] = 1;
    m_tiles[10 * m_width + 11] = 1;
    m_tiles[11 * m_width + 10] = 1;
    m_tiles[11 * m_width + 11] = 1;
}
