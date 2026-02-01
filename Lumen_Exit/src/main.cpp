#include <SFML/Graphics.hpp>
#include <iostream>
#include "Menu.h"

enum class GameState
{
	MENU,
	PLAYING,
	PAUSED
};

int main()
{
	// Параметры окна
	const int SCREEN_WIDTH = 1280;
	const int SCREEN_HEIGHT = 720;

	// Создание окна
	sf::RenderWindow window(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), "Lumen_Exit()");
	window.setFramerateLimit(60);

	std::cout << "Lumen_Exit() initialized successfully!" << std::endl;
	std::cout << "Use Arrow Keys to navigate, Enter to select, ESC to exit" << std::endl;

	// Состояние игры
	GameState gameState = GameState::MENU;

	// Создание меню
	Menu menu(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));

	// Главный игровой цикл
	while (window.isOpen())
	{
		// Обработка событий
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (gameState == GameState::MENU)
				{
					if (event.key.code == sf::Keyboard::Up)
					{
						menu.moveUp();
					}
					else if (event.key.code == sf::Keyboard::Down)
					{
						menu.moveDown();
					}
					else if (event.key.code == sf::Keyboard::Enter)
					{
						int selected = menu.getSelectedItem();
						if (selected == 0) // START GAME
						{
							gameState = GameState::PLAYING;
							std::cout << "Game started!" << std::endl;
						}
						else if (selected == 1) // EXIT
						{
							window.close();
						}
					}
					else if (event.key.code == sf::Keyboard::Escape)
					{
						window.close();
					}
				}
				else if (gameState == GameState::PLAYING)
				{
					if (event.key.code == sf::Keyboard::Escape)
					{
						gameState = GameState::MENU;
						std::cout << "Back to menu" << std::endl;
					}
				}
			}
		}

		// Очистка экрана (темный фон)
		window.clear(sf::Color(10, 10, 10));

		// Отрисовка в зависимости от состояния
		if (gameState == GameState::MENU)
		{
			menu.draw(window);
		}
		else if (gameState == GameState::PLAYING)
		{
			// Временная заглушка для игры
			sf::Font font;
			if (font.loadFromFile("C:\\Windows\\Fonts\\cour.ttf"))
			{
				sf::Text text;
				text.setFont(font);
				text.setString("GAME SCREEN\n\nPress ESC to return to menu");
				text.setCharacterSize(40);
				text.setFillColor(sf::Color::White);
				text.setPosition(400.0f, 300.0f);
				window.draw(text);
			}
		}

		// Отображение
		window.display();
	}

	return 0;
}
