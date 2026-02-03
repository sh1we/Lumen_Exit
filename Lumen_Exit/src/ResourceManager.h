#pragma once
#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <memory>

// Singleton менеджер ресурсов - загружает шрифты/текстуры один раз
class ResourceManager
{
public:
    // Получить единственный экземпляр
    static ResourceManager& getInstance()
    {
        static ResourceManager instance;
        return instance;
    }
    
    // Запрещаем копирование
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    // Загрузка шрифта (кэшируется)
    sf::Font& getFont(const std::string& name = "default");
    
    // Загрузка шрифта из файла
    bool loadFont(const std::string& name, const std::string& filepath);
    
    // Очистка всех ресурсов
    void clear();
    
private:
    ResourceManager();
    ~ResourceManager() = default;
    
    std::unordered_map<std::string, std::unique_ptr<sf::Font>> m_fonts;
    
    // Путь к шрифту по умолчанию
    static constexpr const char* DEFAULT_FONT_PATH = "C:\\Windows\\Fonts\\cour.ttf";
};
