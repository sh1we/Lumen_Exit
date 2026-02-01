#include <SFML/Graphics.hpp>
#include <iostream>
#include "Menu.h"
#include "Map.h"
#include "Player.h"
#include "Raycaster.h"
#include "LoadingScreen.h"

enum class GameState
{
	LOADING,
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

	// Состояние игры
	GameState gameState = GameState::LOADING;

	// Создание загрузочного экрана
	LoadingScreen loadingScreen(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
	
	// Создание меню
	Menu menu(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
	
	// Создание игровых объектов
	Map gameMap(16, 16);
	Player player(8.0f, 8.0f, 0.0f); // Стартовая позиция в центре карты
	Raycaster raycaster(SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Таймер для deltaTime
	sf::Clock clock;

	// Главный игровой цикл
	while (window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		
		// Обработка событий
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (gameState == GameState::LOADING)
				{
					// Загрузочный экран обрабатывает нажатия сам
				}
				else if (gameState == GameState::MENU)
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
		window.clear(sf::Color(0, 0, 0)); // Полностью черный для загрузки

		// Отрисовка в зависимости от состояния
		if (gameState == GameState::LOADING)
		{
			loadingScreen.update(deltaTime);
			loadingScreen.draw(window);
			
			// Переход в меню после завершения загрузки
			if (loadingScreen.isFinished())
			{
				gameState = GameState::MENU;
				std::cout << "Use Arrow Keys to navigate, Enter to select, ESC to exit" << std::endl;
			}
		}
		else if (gameState == GameState::MENU)
		{
			menu.draw(window);
		}
		else if (gameState == GameState::PLAYING)
		{
			// Обновление игрока
			player.update(deltaTime, gameMap);
			
			// Рендеринг 3D мира через raycasting
			window.clear(sf::Color(10, 10, 10));
			raycaster.render(window, player, gameMap);
		}

		// Отображение
		window.display();
	}

	return 0;
}
