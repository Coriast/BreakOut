#ifndef GAME_H
#define GAME_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <filesystem>
#include <tuple>
#include "SpriteRenderer.h"
#include "ResourceManager.h"
#include "GameLevel.h"
#include "GameObject.h"
#include "BallObject.h"
#include "ParticleGenerator.h"
#include "PostProcessor.h"
#include "PowerUp.h"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_WIN
};

enum Direction
{
	UP,
	RIGHT,
	DOWN,
	LEFT
};

typedef std::tuple<bool, Direction, glm::vec2> Collision;

class Game {
public:
	GameState	State;
	bool		Keys[1024];
	unsigned int Width, Height;
	std::vector<GameLevel>	Levels;
	unsigned int			Level;
	std::vector<PowerUp>	PowerUps;

	Game(unsigned int width, unsigned int height);
	~Game();
	// Inicializa o Estado de Jogo (carrega Shaders, texturas, levels)
	void init();
	// Game Loop
	void ProcessInput(float dt);
	void Update(float dt);
	void Render();
	void DoCollisions();
	void SpawnPowerUps(GameObject& block);
	void UpdatePowerUps(float dt);
	
	void ResetLevel();
	void ResetPlayer();
};

#endif