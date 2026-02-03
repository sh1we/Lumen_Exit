#include <SFML/Graphics.hpp>
#include <iostream>
#include "Config.h"
#include "GameManager.h"
#include "../ui/Menu.h"
#include "../ui/LoadingScreen.h"
#include "../ui/Minimap.h"
#include "../ui/VictoryScreen.h"
#include "../ui/HUD.h"
#include "../ui/SettingsMenu.h"
#include "../world/Map.h"
#include "../world/Player.h"
#include "../rendering/Raycaster.h"
#include "../rendering/LightSystem.h"
#include "../rendering/PostProcessing.h"
#include "../utils/ResourceManager.h"

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
	GameConfig config;
	config.loadFromFile("config.txt");

	sf::Uint32 style = config.fullscreen ? sf::Style::Fullscreen : sf::Style::Close;
	sf::RenderWindow window(sf::VideoMode(config.screenWidth, config.screenHeight), "Lumen_Exit()", style);
	window.setFramerateLimit(config.targetFPS);

	std::cout << "Lumen_Exit() initialized successfully!" << std::endl;
	std::cout << "Resolution: " << config.screenWidth << "x" << config.screenHeight << std::endl;
	std::cout << "Target FPS: " << config.targetFPS << std::endl;
	std::cout << "Fullscreen: " << (config.fullscreen ? "YES" : "NO") << std::endl;

	GameState gameState = GameState::LOADING;
	GameState previousState = GameState::LOADING;

	LoadingScreen* loadingScreen = new LoadingScreen(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
	Menu* menu = new Menu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
	
	// mouse stuff
	bool mouseControlEnabled = true;
	sf::Vector2i lastMousePos;
	bool firstMouse = true;
	
	SettingsMenu* settingsMenu = new SettingsMenu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight), config);
	
	// track settings for hot-reload
	int lastScreenWidth = config.screenWidth;
	int lastScreenHeight = config.screenHeight;
	int lastTargetFPS = config.targetFPS;
	bool lastFullscreen = config.fullscreen;
	LightingQuality lastLightingQuality = config.lightingQuality;
	
	GameManager gameManager(config);
	VictoryScreen* victoryScreen = nullptr;
	
	bool showMinimap = false;
	bool tabPressed = false;
	bool escPressed = false;
	bool fPressed = false;
	
	sf::Clock clock;
	float gameTime = 0.0f;

	while (window.isOpen())
	{
		float deltaTime = clock.restart().asSeconds();
		
		// pause when alt-tabbed
		if (!window.hasFocus() && gameState == GameState::PLAYING)
		{
			deltaTime = 0.0f;
		}
		
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			
			// text input for seed
			if (event.type == sf::Event::TextEntered && gameState == GameState::SETTINGS)
			{
				settingsMenu->handleTextInput(event.text.unicode);
			}

			if (event.type == sf::Event::KeyPressed)
			{
				if (gameState == GameState::LOADING)
				{
				}
				else if (gameState == GameState::SETTINGS)
				{
					settingsMenu->handleInput(event.key.code);
					
					if (event.key.code == sf::Keyboard::Escape && !escPressed && !settingsMenu->isEditingSeed())
					{
						gameState = previousState;
						escPressed = true;
					}
				}
				else if (gameState == GameState::MENU)
				{
					if (event.key.code == sf::Keyboard::Up)
					{
						menu->moveUp();
					}
					else if (event.key.code == sf::Keyboard::Down)
					{
						menu->moveDown();
					}
					else if (event.key.code == sf::Keyboard::Enter)
					{
						int selected = menu->getSelectedItem();
						
						if (menu->isInGameMode())
						{
							if (selected == 0) // CONTINUE
							{
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "Game continued!" << std::endl;
							}
							else if (selected == 1) // NEW GAME
							{
								gameManager.createNewGame();
								
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
							if (selected == 0) // START GAME
							{
								if (!gameManager.isInitialized())
								{
									gameManager.createNewGame();
									gameTime = 0.0f;
								}
								
								gameState = GameState::PLAYING;
								menu->setInGameMode(true);
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
						menu->setInGameMode(true);
						window.setMouseCursorVisible(true);
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
						if (gameManager.getLightSystem() != nullptr)
						{
							gameManager.getLightSystem()->setFlashlightEnabled(!gameManager.getLightSystem()->isFlashlightEnabled());
							fPressed = true;
							std::cout << "Flashlight " << (gameManager.getLightSystem()->isFlashlightEnabled() ? "ON" : "OFF") << std::endl;
						}
					}
				}
			}
			
			// mouse clicks in menu
			if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
			{
				if (gameState == GameState::MENU)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					if (menu->handleMouseClick(mousePos))
					{
						int selected = menu->getSelectedItem();
						
						if (menu->isInGameMode())
						{
							if (selected == 0) // CONTINUE
							{
								gameState = GameState::PLAYING;
								window.setMouseCursorVisible(false);
								firstMouse = true;
								std::cout << "Game continued!" << std::endl;
							}
							else if (selected == 1) // NEW GAME
							{
								gameManager.createNewGame();
								
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
							if (selected == 0) // START GAME
							{
								if (!gameManager.isInitialized())
								{
									gameManager.createNewGame();
									gameTime = 0.0f;
								}
								
								gameState = GameState::PLAYING;
								menu->setInGameMode(true);
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
					settingsMenu->handleMouseClick(mousePos);
				}
			}
			
			if (event.type == sf::Event::MouseMoved)
			{
				if (gameState == GameState::MENU)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					menu->handleMouseMove(mousePos);
				}
				else if (gameState == GameState::SETTINGS)
				{
					sf::Vector2i mousePos = sf::Mouse::getPosition(window);
					bool isPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
					settingsMenu->handleMouseMove(mousePos, isPressed);
				}
			}
			
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
		
		// hot-reload settings
		bool needsWindowRecreate = (config.screenWidth != lastScreenWidth || 
		                            config.screenHeight != lastScreenHeight || 
		                            config.fullscreen != lastFullscreen);
		bool needsFPSUpdate = (config.targetFPS != lastTargetFPS);
		
		if (needsWindowRecreate)
		{
			std::cout << "Applying new video settings..." << std::endl;
			
			sf::Uint32 newStyle = config.fullscreen ? sf::Style::Fullscreen : sf::Style::Close;
			window.create(sf::VideoMode(config.screenWidth, config.screenHeight), "Lumen_Exit()", newStyle);
			window.setFramerateLimit(config.targetFPS);
			
			if (gameState == GameState::PLAYING || gameState == GameState::MENU)
			{
				window.setMouseCursorVisible(gameState != GameState::PLAYING);
			}
			
			// recreate UI with new dimensions
			delete loadingScreen;
			delete menu;
			delete settingsMenu;
			
			loadingScreen = new LoadingScreen(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
			menu = new Menu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight));
			settingsMenu = new SettingsMenu(static_cast<float>(config.screenWidth), static_cast<float>(config.screenHeight), config);
			
			// keep menu state
			if (gameManager.isInitialized())
			{
				menu->setInGameMode(true);
			}
			
			lastScreenWidth = config.screenWidth;
			lastScreenHeight = config.screenHeight;
			lastFullscreen = config.fullscreen;
			lastTargetFPS = config.targetFPS;
			
			std::cout << "New resolution: " << config.screenWidth << "x" << config.screenHeight << std::endl;
		}
		else if (needsFPSUpdate)
		{
			window.setFramerateLimit(config.targetFPS);
			lastTargetFPS = config.targetFPS;
			std::cout << "FPS limit set to: " << config.targetFPS << std::endl;
		}
		
		// hot-reload lighting quality
		if (config.lightingQuality != lastLightingQuality)
		{
			if (gameManager.isInitialized() && gameManager.getRaycaster() != nullptr)
			{
				gameManager.getRaycaster()->setLightingQuality(config.lightingQuality);
				std::cout << "Lighting quality updated" << std::endl;
			}
			lastLightingQuality = config.lightingQuality;
		}

		window.clear(sf::Color(0, 0, 0));

		if (gameState == GameState::LOADING)
		{
			loadingScreen->update(deltaTime);
			loadingScreen->draw(window);
			
			if (loadingScreen->isFinished())
			{
				gameState = GameState::MENU;
				std::cout << "Use Arrow Keys to navigate, Enter to select, ESC to exit" << std::endl;
			}
		}
		else if (gameState == GameState::MENU)
		{
			menu->draw(window);
		}
		else if (gameState == GameState::SETTINGS)
		{
			settingsMenu->draw(window);
		}
		else if (gameState == GameState::PLAYING)
		{
			if (window.hasFocus() && gameManager.isInitialized())
			{
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
						gameManager.getPlayer()->handleMouseMovement(deltaX, config.mouseSensitivity);
					}
					
					sf::Vector2i center(config.screenWidth / 2, config.screenHeight / 2);
					sf::Mouse::setPosition(center, window);
					lastMousePos = center;
				}
				
				gameManager.getPlayer()->update(deltaTime, *gameManager.getMap());
				gameTime += deltaTime;
				
				bool inSafeRoom = gameManager.getPlayer()->isInRoom(*gameManager.getMap());
				gameManager.getLightSystem()->updateFlashlight(deltaTime, gameManager.getLightSystem()->isFlashlightEnabled(), inSafeRoom);
				
				// update visible lights for frustum culling
				gameManager.getLightSystem()->updateVisibleLights(*gameManager.getPlayer());
				
				// win condition
				if (gameManager.getPlayer()->hasReachedExit())
				{
					std::cout << "\n==================================" << std::endl;
					std::cout << "    EXIT FOUND! YOU ESCAPED!" << std::endl;
					std::cout << "    Time: " << static_cast<int>(gameTime) << " seconds" << std::endl;
					std::cout << "    Seed: " << gameManager.getMap()->getSeed() << std::endl;
					std::cout << "==================================" << std::endl;
					
					config.updateBestTime(gameTime);
					config.saveToFile("config.txt");
					
					victoryScreen = new VictoryScreen(
						static_cast<float>(config.screenWidth), 
						static_cast<float>(config.screenHeight),
						gameTime,
						config.bestTime,
						gameManager.getMap()->getSeed()
					);
					gameState = GameState::VICTORY;
					window.setMouseCursorVisible(true);
				}
			}
			
			if (gameManager.isInitialized())
			{
				window.clear(sf::Color(10, 10, 10));
				gameManager.getRaycaster()->render(window, *gameManager.getPlayer(), *gameManager.getMap(), *gameManager.getLightSystem());
				
				gameManager.getPostProcessing()->applyEffects(window, 0.0f, gameManager.getLightSystem()->getFlashlightBattery());
				gameManager.getHUD()->draw(window, *gameManager.getPlayer(), gameTime, *gameManager.getLightSystem(), gameManager.getMap()->getSeed());
				
				if (showMinimap)
				{
					gameManager.getMinimap()->draw(window, *gameManager.getPlayer(), *gameManager.getMap());
				}
			}
		}
		else if (gameState == GameState::VICTORY)
		{
			if (victoryScreen != nullptr)
			{
				victoryScreen->update(deltaTime);
				victoryScreen->draw(window);
				
				if (victoryScreen->isFinished())
				{
					gameState = GameState::MENU;
					
					gameManager.cleanup();
					delete victoryScreen;
					victoryScreen = nullptr;
					gameTime = 0.0f;
					menu->setInGameMode(false);
					
					std::cout << "Returning to menu..." << std::endl;
				}
			}
		}

		window.display();
	}
	
	config.saveToFile("config.txt");
	
	delete loadingScreen;
	delete menu;
	delete settingsMenu;
	delete victoryScreen;

	return 0;
}
