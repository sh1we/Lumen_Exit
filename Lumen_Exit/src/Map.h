#pragma once
#include <vector>

class Map
{
public:
    Map(int width, int height);
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTile(int x, int y) const;
    bool isWall(int x, int y) const;
    
private:
    void generateTestMap();
    
    int m_width;
    int m_height;
    std::vector<int> m_tiles; // 0 = пустота, 1 = стена
};
