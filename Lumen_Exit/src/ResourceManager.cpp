#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager()
{
    loadFont("default", DEFAULT_FONT_PATH);
}

sf::Font& ResourceManager::getFont(const std::string& name)
{
    auto it = m_fonts.find(name);
    if (it != m_fonts.end())
    {
        return *it->second;
    }
    
    std::cerr << "Font '" << name << "' not found, loading default..." << std::endl;
    return *m_fonts["default"];
}

bool ResourceManager::loadFont(const std::string& name, const std::string& filepath)
{
    if (m_fonts.find(name) != m_fonts.end())
    {
        std::cout << "Font '" << name << "' already loaded" << std::endl;
        return true;
    }
    
    auto font = std::make_unique<sf::Font>();
    
    if (!font->loadFromFile(filepath))
    {
        std::cerr << "Failed to load font from: " << filepath << std::endl;
        return false;
    }
    
    std::cout << "Font '" << name << "' loaded successfully from: " << filepath << std::endl;
    m_fonts[name] = std::move(font);
    return true;
}

void ResourceManager::clear()
{
    m_fonts.clear();
    std::cout << "All resources cleared" << std::endl;
}
