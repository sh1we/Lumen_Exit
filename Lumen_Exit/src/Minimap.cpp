#include "Minimap.h"
#include "Map.h"
#include "Player.h"
#include <cmath>

Minimap::Minimap(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_scale(10.0f)  // 10px per tile
{
}

void Minimap::draw(sf::RenderWindow& window, const Player& player, const Map& map)
{
    int mapWidth = map.getWidth();
    int mapHeight = map.getHeight();
    
    sf::VertexArray tiles(sf::Quads);
    
    for (int y = 0; y < mapHeight; ++y)
    {
        for (int x = 0; x < mapWidth; ++x)
        {
            float xPos = 20.0f + x * m_scale;
            float yPos = 20.0f + y * m_scale;
            float tileSize = m_scale - 1.0f;
            
            sf::Color tileColor;
            
            if (!player.hasVisited(x, y))
            {
                tileColor = sf::Color(15, 15, 15);  // unexplored
            }
            else if (map.isWall(x, y))
            {
                tileColor = sf::Color(180, 180, 180);
            }
            else if (map.isInRoom(x, y))
            {
                if (map.isInExitRoom(x, y))
                {
                    tileColor = sf::Color(255, 215, 0);  // gold = exit
                }
                else
                {
                    tileColor = sf::Color(80, 255, 80);  // green = safe room
                }
            }
            else
            {
                tileColor = sf::Color(90, 90, 90);  // corridor
            }
            
            tiles.append(sf::Vertex(sf::Vector2f(xPos, yPos), tileColor));
            tiles.append(sf::Vertex(sf::Vector2f(xPos + tileSize, yPos), tileColor));
            tiles.append(sf::Vertex(sf::Vector2f(xPos + tileSize, yPos + tileSize), tileColor));
            tiles.append(sf::Vertex(sf::Vector2f(xPos, yPos + tileSize), tileColor));
        }
    }
    
    // background
    sf::RectangleShape background(sf::Vector2f(
        mapWidth * m_scale + 20.0f,
        mapHeight * m_scale + 20.0f
    ));
    background.setPosition(10.0f, 10.0f);
    background.setFillColor(sf::Color(10, 10, 10, 240));
    window.draw(background);
    
    window.draw(tiles);
    
    // border
    sf::RectangleShape border(sf::Vector2f(
        mapWidth * m_scale + 20.0f,
        mapHeight * m_scale + 20.0f
    ));
    border.setPosition(10.0f, 10.0f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(255, 255, 255));
    border.setOutlineThickness(3.0f);
    window.draw(border);
    
    // player dot
    sf::CircleShape playerDot(m_scale / 2.0f);
    playerDot.setPosition(
        20.0f + player.getX() * m_scale - m_scale / 2.0f,
        20.0f + player.getY() * m_scale - m_scale / 2.0f
    );
    playerDot.setFillColor(sf::Color(255, 255, 0));
    window.draw(playerDot);
    
    // facing direction
    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(20.0f + player.getX() * m_scale, 20.0f + player.getY() * m_scale), sf::Color(255, 255, 0)),
        sf::Vertex(sf::Vector2f(
            20.0f + player.getX() * m_scale + player.getDirX() * m_scale * 2.0f,
            20.0f + player.getY() * m_scale + player.getDirY() * m_scale * 2.0f
        ), sf::Color(255, 255, 0))
    };
    window.draw(line, 2, sf::Lines);
}
