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
#include "SettingsMenu.h"

enum class GameState
{
	LOADING,
	MENU,
	SETTINGS,
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
	GameState previousState = GameState::LOADING; // Для возврата из настроек

	// Создание загрузочного экрана
	LoadingScreen loadingScreen(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
	
	// Создание меню
	Menu menu(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT));
	
	// Управление мышкой (объявляем ДО создания settingsMenu)
	bool mouseControlEnabled = true;
	float mouseSensitivity = 0.001f; // Уменьшил чувствительность (было 0.002)
	sf::Vector2i lastMousePos;
	bool firstMouse = true;
	
	// Создание меню настроек (после объявления mouseSensitivity)
	SettingsMenu settingsMenu(static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT), mouseSensitivity);
	
	// Игровые объекты (создаются только при старте игры)
	Map* gameMap = nullptr;
	Player* player = nullptr;
	Raycaster* raycaster = nullptr;
	Minimap* minimap = nullptr;
	VictoryScreen* victoryScreen = nullptr;
	HUD* hud = nullptr;
	bool showMinimap = false;
	bool tabPressed = false; // Для debounce клавиши Tab
	bool escPressed = false; // Для debounce клавиши ESC в настройках
	
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
				else if (gameState == GameState::SETTINGS)
				{
					settingsMenu.handleInput(event.key.code);
					
					if (event.key.code == sf::Keyboard::Escape && !escPressed)
					{
						gameState = previousState;
						escPressed = true;
					}
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
							firstMouse = true; // Сбрасываем флаг мыши
							window.setMouseCursorVisible(false); // Скрываем курсор
							std::cout << "Game started!" << std::endl;
						}
						else if (selected == 1) // SETTINGS
						{
							previousState = GameState::MENU;
							gameState = GameState::SETTINGS;
						}
						else if (selected == 2) // EXIT
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
						window.setMouseCursorVisible(true); // Показываем курсор
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
			
			// Обработка кликов мыши в меню
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				if (gameState == GameState::MENU)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					if (menu.handleMouseClick(mousePos))
					{
						// Обрабатываем выбор как Enter
						int selected = menu.getSelectedItem();
						if (selected == 0) // START GAME
						{
							// Создаем игровые объекты только при старте игры
							if (gameMap == nullptr)
							{
								gameMap = new Map(51, 51);
								
								float spawnX, spawnY;
								gameMap->getSpawnPosition(spawnX, spawnY);
								player = new Player(spawnX, spawnY, 0.0f);
								
								raycaster = new Raycaster(SCREEN_WIDTH, SCREEN_HEIGHT);
								minimap = new Minimap(SCREEN_WIDTH, SCREEN_HEIGHT);
								hud = new HUD(SCREEN_WIDTH, SCREEN_HEIGHT);
							}
							
							gameState = GameState::PLAYING;
							gameTime = 0.0f;
							firstMouse = true;
							window.setMouseCursorVisible(false);
							std::cout << "Game started!" << std::endl;
						}
						else if (selected == 1) // SETTINGS
						{
							previousState = GameState::MENU;
							gameState = GameState::SETTINGS;
						}
						else if (selected == 2) // EXIT
						{
							window.close();
						}
					}
				}
				else if (gameState == GameState::SETTINGS)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					settingsMenu.handleMouseClick(mousePos);
				}
			}
			
			// Обработка движения мыши в меню
			if (event.type == sf::Event::MouseMoved)
			{
				if (gameState == GameState::MENU)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					menu.handleMouseMove(mousePos);
				}
				else if (gameState == GameState::SETTINGS)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					bool isPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
					settingsMenu.handleMouseMove(mousePos, isPressed);
				}
			}
			
			// Отслеживаем отпускание Tab и ESC
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Tab)
				{
					tabPressed = false;
				}
				else if (event.key.code == sf::Keyboard::Escape)
				{
					escPressed = false;
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
		else if (gameState == GameState::SETTINGS)
		{
			settingsMenu.draw(window);
		}
		else if (gameState == GameState::PLAYING)
		{
			// Обновление игрока только если окно в фокусе
			if (window.hasFocus() && player != nullptr && gameMap != nullptr)
			{
				// Управление мышкой
				if (mouseControlEnabled)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					
					if (firstMouse)
					{
						lastMousePos = mousePos;
						firstMouse = false;
					}
					
					float deltaX = static_cast<float>(mousePos.x - lastMousePos.x);
					
					if (deltaX != 0.0f)
					{
						player->handleMouseMovement(deltaX, mouseSensitivity);
					}
					
					// Возвращаем курсор в центр окна для непрерывного вращения
					sf::Vector2i center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
					sf::Mouse::setPosition(center, window);
					lastMousePos = center;
				}
				
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
					window.setMouseCursorVisible(true); // Показываем курсор
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
