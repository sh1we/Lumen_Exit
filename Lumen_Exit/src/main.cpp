#include <SFML/Graphics.hpp>
#include <iostream>
#include "Menu.h"
#include "Map.h"
#include "Player.h"
#include "Raycaster.h"
#include "LoadingScreen.h"
#include "Minimap.h"
#include "VictoryScreen.h"
#include "HUD.h"

enum class GameState
{
	LOADING,
	MENU,
	PLAYING,
	VICTORY,
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
	
	// Игровые объекты (создаются только при старте игры)
	Map* gameMap = nullptr;
	Player* player = nullptr;
	Raycaster* raycaster = nullptr;
	Minimap* minimap = nullptr;
	VictoryScreen* victoryScreen = nullptr;
	HUD* hud = nullptr;
	bool showMinimap = false;
	bool tabPressed = false; // Для debounce клавиши Tab
	
	// Таймер для deltaTime и игрового времени
	sf::Clock clock;
	float gameTime = 0.0f; // Время прохождения уровня

	// Главный игровой цикл
	while (window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		
		// Проверка фокуса окна - игра на паузе если окно свернуто
		if (!window.hasFocus() && gameState == GameState::PLAYING)
		{
			deltaTime = 0.0f; // Останавливаем время
		}
		
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
							// Создаем игровые объекты только при старте игры
							if (gameMap == nullptr)
							{
								gameMap = new Map(51, 51); // Увеличили карту до 51x51
								
								float spawnX, spawnY;
								gameMap->getSpawnPosition(spawnX, spawnY);
								player = new Player(spawnX, spawnY, 0.0f);
								
								raycaster = new Raycaster(SCREEN_WIDTH, SCREEN_HEIGHT);
								minimap = new Minimap(SCREEN_WIDTH, SCREEN_HEIGHT);
								hud = new HUD(SCREEN_WIDTH, SCREEN_HEIGHT);
							}
							
							gameState = GameState::PLAYING;
							gameTime = 0.0f; // Сбрасываем таймер
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
					else if (event.key.code == sf::Keyboard::Tab && !tabPressed)
					{
						showMinimap = !showMinimap;
						tabPressed = true;
						std::cout << "Minimap " << (showMinimap ? "ON" : "OFF") << std::endl;
					}
				}
			}
			
			// Отслеживаем отпускание Tab
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Tab)
				{
					tabPressed = false;
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
			// Обновление игрока только если окно в фокусе
			if (window.hasFocus() && player != nullptr && gameMap != nullptr)
			{
				player->update(deltaTime, *gameMap);
				gameTime += deltaTime; // Считаем время прохождения
				
				// Проверяем, достиг ли игрок выхода
				if (player->hasReachedExit())
				{
					std::cout << "\n==================================" << std::endl;
					std::cout << "    EXIT FOUND! YOU ESCAPED!" << std::endl;
					std::cout << "    Time: " << static_cast<int>(gameTime) << " seconds" << std::endl;
					std::cout << "==================================" << std::endl;
					
					// Создаем экран победы
					victoryScreen = new VictoryScreen(
						static_cast<float>(SCREEN_WIDTH), 
						static_cast<float>(SCREEN_HEIGHT),
						gameTime
					);
					gameState = GameState::VICTORY;
				}
			}
			
			// Рендеринг 3D мира через raycasting
			if (raycaster != nullptr && player != nullptr && gameMap != nullptr)
			{
				window.clear(sf::Color(10, 10, 10));
				raycaster->render(window, *player, *gameMap);
				
				// HUD рисуется поверх игры
				if (hud != nullptr)
				{
					hud->draw(window, *player, gameTime);
				}
				
				// Миникарта рисуется ПОВЕРХ всего (если включена)
				if (showMinimap && minimap != nullptr)
				{
					minimap->draw(window, *player, *gameMap);
				}
			}
		}
		else if (gameState == GameState::VICTORY)
		{
			if (victoryScreen != nullptr)
			{
				victoryScreen->update(deltaTime);
				victoryScreen->draw(window);
				
				// Переход в меню после завершения экрана победы
				if (victoryScreen->isFinished())
				{
					gameState = GameState::MENU;
					
					// Сбрасываем игру для новой попытки
					delete gameMap;
					delete player;
					delete raycaster;
					delete minimap;
					delete victoryScreen;
					delete hud;
					gameMap = nullptr;
					player = nullptr;
					raycaster = nullptr;
					minimap = nullptr;
					victoryScreen = nullptr;
					hud = nullptr;
					gameTime = 0.0f;
					
					std::cout << "Returning to menu..." << std::endl;
				}
			}
		}

		// Отображение
		window.display();
	}
	
	// Очистка памяти
	delete gameMap;
	delete player;
	delete raycaster;
	delete minimap;
	delete victoryScreen;
	delete hud;

	return 0;
}
