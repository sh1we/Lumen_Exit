#pragma once
#include <vector>
#include <random>

struct Room
{
    int x, y;
    int width, height;
    bool isExit = false;
    int centerX() const { return x + width / 2; }
    int centerY() const { return y + height / 2; }
};

class Map
{
public:
    Map(int width, int height, unsigned int seed = 0);
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getTile(int x, int y) const;
    bool isWall(int x, int y) const;
    bool isInRoom(int x, int y) const;
    bool isInExitRoom(int x, int y) const;
    
    void getSpawnPosition(float& outX, float& outY) const;
    const std::vector<Room>& getRooms() const { return m_rooms; }
    
    unsigned int getSeed() const { return m_seed; }
    
private:
    void generateMaze(unsigned int seed);
    void recursiveBacktracker(int x, int y, std::mt19937& rng);
    void addRooms(std::mt19937& rng, int roomCount);
    bool isValidCell(int x, int y) const;
    
    int m_width;
    int m_height;
    std::vector<int> m_tiles;
    
    std::vector<Room> m_rooms;
    int m_spawnX;
    int m_spawnY;
    unsigned int m_seed;
};
