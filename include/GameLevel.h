#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GameObject.h"
#include "SpriteRenderer.h"
#include "ResourceManager.h"

class GameLevel
{
public:
	std::vector<GameObject> Bricks;

	GameLevel() { }
	// loads level from file
	void Load(const char* file, unsigned int levelWidth, unsigned int levelHeight);
	//render level
	void Draw(SpriteRenderer& renderer);
	// check if level is completed (all non-solid tiles are destroyed)
	bool IsCompleted();

private:
	void init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight);
};
