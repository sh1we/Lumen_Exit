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
#include "LightSystem.h"
#include "PostProcessing.h"
#include "Config.h"

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
	// Загрузка конфигурации
	GameConfig config;
	config.loadFromFile("config.txt");

	// Создание окна с настройками из конфига
	sf::Uint32 style = config.fullscreen ? sf::Style::Fullscreen : sf::Style::Close;
	sf::RenderWindow window(sf::VideoMode(config.screenWidth, config.screenHeight), "Lumen_Exit()", style);
	window.setFramerateLimit(config.targetFPS);

	std::cout << "Lumen_Exit() initialized successfully!" << std::endl;
	std::cout << "Resolution: " << config.screenWidth << "x" << config.screenHeight << std::endl;
	std::cout << "Target FPS: " << config.targetFPS << std::endl;
	std::cout << "Fullscreen: " << (config.fullscreen ? "YES" : "NO") << std::endl;

	// Состояние игры
	GameState gameState = GameState::LOADING;
	GameState previousState = GameState::LOADING; // Для возврата из настроек

	// Создание загрузочного экрана
	LoadingScreen loadingScreen(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
	
	// Создание меню
	Menu menu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
	
	// Управление мышкой
	bool mouseControlEnabled = true;
	sf::Vector2i lastMousePos;
	bool firstMouse = true;
	
	// Создание меню настроек с передачей конфига
	SettingsMenu settingsMenu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight), config);
	
	// Игровые объекты (создаются только при старте игры)
	Map* gameMap = nullptr;
	Player* player = nullptr;
	Raycaster* raycaster = nullptr;
	Minimap* minimap = nullptr;
	VictoryScreen* victoryScreen = nullptr;
	HUD* hud = nullptr;
	LightSystem* lightSystem = nullptr;
	PostProcessing* postProcessing = nullptr;
	bool showMinimap = false;
	bool tabPressed = false; // Для debounce клавиши Tab
	bool escPressed = false; // Для debounce клавиши ESC в настройках
	bool fPressed = false;   // Для debounce клавиши F (фонарик)
	
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
						
						if (menu.isInGameMode())
						{
							// Меню во время игры: CONTINUE, NEW GAME, SETTINGS, EXIT
							if (selected == 0) // CONTINUE
							{
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "Game continued!" << std::endl;
							}
							else if (selected == 1) // NEW GAME
							{
								// Удаляем старую игру
								delete gameMap;
								delete player;
								delete raycaster;
								delete minimap;
								delete hud;
								delete lightSystem;
								delete postProcessing;
								
								// Создаем новую игру
								gameMap = new Map(51, 51);
								
								float spawnX, spawnY;
								gameMap->getSpawnPosition(spawnX, spawnY);
								player = new Player(spawnX, spawnY, 0.0f);
								
								raycaster = new Raycaster(config.screenWidth, config.screenHeight);
								minimap = new Minimap(config.screenWidth, config.screenHeight);
								hud = new HUD(config.screenWidth, config.screenHeight);
								
								lightSystem = new LightSystem();
								lightSystem->addRoomLights(*gameMap);
								
								postProcessing = new PostProcessing(config.screenWidth, config.screenHeight);
								
								gameTime = 0.0f;
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "New game started!" << std::endl;
							}
							else if (selected == 2) // SETTINGS
							{
								previousState = GameState::MENU;
								gameState = GameState::SETTINGS;
							}
							else if (selected == 3) // EXIT
							{
								window.close();
							}
						}
						else
						{
							// Меню до начала игры: START GAME, SETTINGS, EXIT
							if (selected == 0) // START GAME
							{
								// Создаем игровые объекты только при старте игры
								if (gameMap == nullptr)
								{
									gameMap = new Map(51, 51); // Увеличили карту до 51x51
									
									float spawnX, spawnY;
									gameMap->getSpawnPosition(spawnX, spawnY);
									player = new Player(spawnX, spawnY, 0.0f);
									
									raycaster = new Raycaster(config.screenWidth, config.screenHeight);
									minimap = new Minimap(config.screenWidth, config.screenHeight);
									hud = new HUD(config.screenWidth, config.screenHeight);
									
									// Создаем систему освещения и добавляем свет в комнаты
									lightSystem = new LightSystem();
									lightSystem->addRoomLights(*gameMap);
									
									// Создаем пост-обработку
									postProcessing = new PostProcessing(config.screenWidth, config.screenHeight);
									
									// ИСПРАВЛЕНИЕ: Сбрасываем таймер только при создании новой игры
									gameTime = 0.0f;
								}
								
								gameState = GameState::PLAYING;
								menu.setInGameMode(true); // Переключаем меню в режим "во время игры"
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
						menu.setInGameMode(true); // Показываем меню "во время игры"
						window.setMouseCursorVisible(true); // Показываем курсор
						std::cout << "Back to menu" << std::endl;
					}
					else if (event.key.code == sf::Keyboard::Tab && !tabPressed)
					{
						showMinimap = !showMinimap;
						tabPressed = true;
						std::cout << "Minimap " << (showMinimap ? "ON" : "OFF") << std::endl;
					}
					else if (event.key.code == sf::Keyboard::F && !fPressed)
					{
						if (lightSystem != nullptr)
						{
							lightSystem->setFlashlightEnabled(!lightSystem->isFlashlightEnabled());
							fPressed = true;
							std::cout << "Flashlight " << (lightSystem->isFlashlightEnabled() ? "ON" : "OFF") << std::endl;
						}
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
						
						if (menu.isInGameMode())
						{
							// Меню во время игры
							if (selected == 0) // CONTINUE
							{
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "Game continued!" << std::endl;
							}
							else if (selected == 1) // NEW GAME
							{
								// Удаляем старую игру
								delete gameMap;
								delete player;
								delete raycaster;
								delete minimap;
								delete hud;
								delete lightSystem;
								delete postProcessing;
								
								// Создаем новую игру
								gameMap = new Map(51, 51);
								
								float spawnX, spawnY;
								gameMap->getSpawnPosition(spawnX, spawnY);
								player = new Player(spawnX, spawnY, 0.0f);
								
								raycaster = new Raycaster(config.screenWidth, config.screenHeight);
								minimap = new Minimap(config.screenWidth, config.screenHeight);
								hud = new HUD(config.screenWidth, config.screenHeight);
								
								lightSystem = new LightSystem();
								lightSystem->addRoomLights(*gameMap);
								
								postProcessing = new PostProcessing(config.screenWidth, config.screenHeight);
								
								gameTime = 0.0f;
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "New game started!" << std::endl;
							}
							else if (selected == 2) // SETTINGS
							{
								previousState = GameState::MENU;
								gameState = GameState::SETTINGS;
							}
							else if (selected == 3) // EXIT
							{
								window.close();
							}
						}
						else
						{
							// Меню до начала игры
							if (selected == 0) // START GAME
							{
								// Создаем игровые объекты только при старте игры
								if (gameMap == nullptr)
								{
									gameMap = new Map(51, 51);
									
									float spawnX, spawnY;
									gameMap->getSpawnPosition(spawnX, spawnY);
									player = new Player(spawnX, spawnY, 0.0f);
									
									raycaster = new Raycaster(config.screenWidth, config.screenHeight);
									minimap = new Minimap(config.screenWidth, config.screenHeight);
									hud = new HUD(config.screenWidth, config.screenHeight);
									
									// Создаем систему освещения
									lightSystem = new LightSystem();
									lightSystem->addRoomLights(*gameMap);
									
									// Создаем пост-обработку
									postProcessing = new PostProcessing(config.screenWidth, config.screenHeight);
									
									// ИСПРАВЛЕНИЕ: Сбрасываем таймер только при создании новой игры
									gameTime = 0.0f;
								}
								
								gameState = GameState::PLAYING;
								menu.setInGameMode(true);
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
				else if (event.key.code == sf::Keyboard::F)
				{
					fPressed = false;
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
			if (window.hasFocus() && player != nullptr && gameMap != nullptr && lightSystem != nullptr)
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
						player->handleMouseMovement(deltaX, config.mouseSensitivity);
					}
					
					// Возвращаем курсор в центр окна для непрерывного вращения
					sf::Vector2i center(config.screenWidth / 2, config.screenHeight / 2);
					sf::Mouse::setPosition(center, window);
					lastMousePos = center;
				}
				
				player->update(deltaTime, *gameMap);
				gameTime += deltaTime; // Считаем время прохождения
				
				// Проверяем, находится ли игрок в safe комнате
				bool inSafeRoom = player->isInRoom(*gameMap);
				
				// Обновляем фонарик (зарядка в комнате, разрядка при использовании)
				lightSystem->updateFlashlight(deltaTime, lightSystem->isFlashlightEnabled(), inSafeRoom);
				
				// Проверяем, достиг ли игрок выхода
				if (player->hasReachedExit())
				{
					std::cout << "\n==================================" << std::endl;
					std::cout << "    EXIT FOUND! YOU ESCAPED!" << std::endl;
					std::cout << "    Time: " << static_cast<int>(gameTime) << " seconds" << std::endl;
					std::cout << "==================================" << std::endl;
					
					// Обновляем лучшее время
					config.updateBestTime(gameTime);
					config.saveToFile("config.txt");
					
					// Создаем экран победы
					victoryScreen = new VictoryScreen(
						static_cast<float>(config.screenWidth), 
						static_cast<float>(config.screenHeight),
						gameTime,
						config.bestTime
					);
					gameState = GameState::VICTORY;
					window.setMouseCursorVisible(true); // Показываем курсор
				}
			}
			
			// Рендеринг 3D мира через raycasting
			if (raycaster != nullptr && player != nullptr && gameMap != nullptr && lightSystem != nullptr && postProcessing != nullptr)
			{
				window.clear(sf::Color(10, 10, 10));
				raycaster->render(window, *player, *gameMap, *lightSystem);
				
				// Применяем пост-обработку (vignette, эффекты)
				postProcessing->applyEffects(window, 0.0f, lightSystem->getFlashlightBattery());
				
				// HUD рисуется поверх игры
				if (hud != nullptr)
				{
					hud->draw(window, *player, gameTime, *lightSystem);
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
					delete lightSystem;
					delete postProcessing;
					gameMap = nullptr;
					player = nullptr;
					raycaster = nullptr;
					minimap = nullptr;
					victoryScreen = nullptr;
					hud = nullptr;
					lightSystem = nullptr;
					postProcessing = nullptr;
					gameTime = 0.0f;
					
					// Переключаем меню обратно в начальный режим
					menu.setInGameMode(false);
					
					std::cout << "Returning to menu..." << std::endl;
				}
			}
		}

		// Отображение
		window.display();
	}
	
	// Сохраняем конфигурацию перед выходом
	config.saveToFile("config.txt");
	
	// Очистка памяти
	delete gameMap;
	delete player;
	delete raycaster;
	delete minimap;
	delete victoryScreen;
	delete hud;
	delete lightSystem;
	delete postProcessing;

	return 0;
}
