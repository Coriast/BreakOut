#include "Game.h"

using namespace std;
using namespace irrklang;

string fileP = ResourceManager::GetAbsolutePath();

const glm::vec2 PLAYER_SIZE(100.0f, 20.0f);
const float PLAYER_VELOCITY(500.0f);

const glm::vec2 INITIAL_BALL_VELOCITY(100.0f, -350.0f);
const float BALL_RADIUS = 12.5f;

SpriteRenderer*		Renderer;
GameObject*			Player;
BallObject*			Ball;
ParticleGenerator*	Particles;
PostProcessor*		Effects;
ISoundEngine*		SoundEngine = createIrrKlangDevice();
TextRenderer*		Text;

float ShakeTime = 0.0f;

bool checkCollision(GameObject& one, GameObject& two);
Collision checkCollision(BallObject& one, GameObject& two);
Direction vectorDirection(glm::vec2 target);
bool ShouldSpawn(unsigned int chance);
void ActivatePowerUp(PowerUp& powerUp);
bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type);


Game::Game(unsigned int width, unsigned int height) : State(GAME_MENU), Keys(), Width(width), Height(height), Level(0), Lives(3) {

}

Game::~Game() {
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
	delete Effects;
	delete Text;
	SoundEngine->drop();
}

void Game::init() {
	ResourceManager::LoadShader("/shaders/sprite.vs", "/shaders/sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("/shaders/particle.vs", "/shaders/particle.frag", nullptr, "particle");	
	ResourceManager::LoadShader("/shaders/postProcessing.vs", "/shaders/postProcessing.frag", nullptr, "postProcessing");

	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width), static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").Use().SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);

	ResourceManager::LoadTexture("/textures/face.png", true, "face");
	ResourceManager::LoadTexture("/textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("/textures/block.png", false, "block");
	ResourceManager::LoadTexture("/textures/blockSolid.png", false, "blockSolid");
	ResourceManager::LoadTexture("/textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("/textures/particle.png", true, "particle");
	ResourceManager::LoadTexture("/textures/powerup_chaos.png", true, "chaos");
	ResourceManager::LoadTexture("/textures/powerup_confuse.png", true, "confuse");
	ResourceManager::LoadTexture("/textures/powerup_increase.png", true, "increase");
	ResourceManager::LoadTexture("/textures/powerup_passThrough.png", true, "passThrough");
	ResourceManager::LoadTexture("/textures/powerup_speed.png", true, "speed");
	ResourceManager::LoadTexture("/textures/powerup_sticky.png", true, "sticky");

	Renderer	= new SpriteRenderer(ResourceManager::GetShader("sprite"));
	Particles	= new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	Effects = new PostProcessor(ResourceManager::GetShader("postProcessing"), this->Width, this->Height);

	GameLevel one, two, three, four;
	one.Load(string(fileP + "/levels/one.lvl").c_str(), this->Width, this->Height / 2);
	two.Load(string(fileP + "/levels/two.lvl").c_str(), this->Width, this->Height / 2);
	three.Load(string(fileP + "/levels/three.lvl").c_str(), this->Width, this->Height / 2);
	four.Load(string(fileP + "/levels/four.lvl").c_str(), this->Width, this->Height / 2);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;

	glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));

	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
	SoundEngine->play2D(string(fileP + "/sounds/breakout.mp3").c_str(), true);

	Text = new TextRenderer(this->Width, this->Height);
	Text->Load(string(fileP + "/fonts/OCRAEXT.TTF").c_str(), 24);
}

void Game::Update(float dt) {
	Ball->Move(dt, this->Width);
	this->DoCollisions();

	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

	this->UpdatePowerUps(dt);

	if (ShakeTime > 0.0f)
	{
		ShakeTime -= dt;
		if (ShakeTime <= 0.0f)
			Effects->Shake = false;
	}

	if (Ball->Position.y >= this->Height)
	{
		--this->Lives;
		if (this->Lives == 0)
		{
			this->ResetLevel();
			this->State = GAME_MENU;
		}
		this->ResetPlayer();
	}

	if (this->State == GAME_ACTIVE && this->Levels[this->Level].IsCompleted())
	{
		this->ResetLevel();
		this->ResetPlayer();
		Effects->Chaos = true;
		this->State = GAME_WIN;
	}
}

void Game::ProcessInput(float dt) {
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;

		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0.0f)
				Player->Position.x -= velocity;
			if (Ball->Stuck && Ball->Position.x >= 0.0f + (PLAYER_SIZE.x / 2.0f - BALL_RADIUS))
				Ball->Position.x -= velocity;
		} 
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
				Player->Position.x += velocity;
			if (Ball->Stuck && Ball->Position.x <= this->Width -Ball->Size.x - (PLAYER_SIZE.x / 2.0f - BALL_RADIUS))
				Ball->Position.x += velocity;
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
	if (this->State == GAME_MENU)
	{
		if (this->Keys[GLFW_KEY_ENTER] && !this->KeysProcessed[GLFW_KEY_ENTER])
		{
			this->State = GAME_ACTIVE;
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
		}
		if (this->Keys[GLFW_KEY_W] && !this->KeysProcessed[GLFW_KEY_W])
		{
			this->Level = (this->Level + 1) % 4;
			this->KeysProcessed[GLFW_KEY_W] = true;
		}
		if (this->Keys[GLFW_KEY_S] && !this->KeysProcessed[GLFW_KEY_S])
		{
			if (this->Level > 0)
				--this->Level;
			else
				this->Level = 3;
			this->KeysProcessed[GLFW_KEY_S] = true;
		}
	}
	if (this->State == GAME_WIN)
	{
		if (this->Keys[GLFW_KEY_ENTER])
		{
			this->KeysProcessed[GLFW_KEY_ENTER] = true;
			Effects->Chaos = false;
			this->State = GAME_MENU;
		}
	}
}

void Game::Render() {
	if (this->State == GAME_ACTIVE || this->State == GAME_MENU)
	{
		Effects->BeginRender();
		{
			Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
			this->Levels[this->Level].Draw(*Renderer);
			Player->Draw(*Renderer);
			Particles->Draw();
			Ball->Draw(*Renderer);
			for (PowerUp& powerUp : this->PowerUps)
			{
				if (!powerUp.Destroyed)
					powerUp.Draw(*Renderer);
			}

			std::stringstream ss; ss << this->Lives;
			Text->RenderText("Lives:" + ss.str(), 5.0f, 5.0f, 1.0f);

			Effects->EndRender();
			Effects->Render(glfwGetTime());
		}

	}
	if (this->State == GAME_MENU)
	{
		Text->RenderText("Press ENTER to start", 250.0f, Height / 2, 1.0f);
		Text->RenderText("Press W or S to select level", 245.0f, Height / 2 + 20.0f, 0.75f);
	}
	if (this->State == GAME_WIN)
	{
		Text->RenderText("TU GANHOU?!?", 320.0, Height / 2 - 20.0, 1.0, glm::vec3(0.0, 1.0, 0.0));
		Text->RenderText("Press ENTER to retry or ESC to quit", 130.0, Height / 2, 1.0, glm::vec3(1.0, 1.0, 0.0));
	}
}

void Game::DoCollisions()
{
	for (GameObject& box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = checkCollision(*Ball, box);
			if (std::get<0>(collision))
			{
				if (!box.IsSolid)
				{
					box.Destroyed = true;
					this->SpawnPowerUps(box);
					SoundEngine->play2D(string(fileP + "/sounds/bleep.mp3").c_str(), false);
				}
				else
				{
					ShakeTime = 0.05f;
					Effects->Shake = true;
					SoundEngine->play2D(string(fileP + "/sounds/solid.wav").c_str(), false);
				}
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if (!(Ball->PassThrough && !box.IsSolid))
				{
					if (dir == LEFT || dir == RIGHT)
					{
						Ball->Velocity.x = -Ball->Velocity.x;
						float penetration = Ball->Radius - std::abs(diff_vector.x);
						if (dir == LEFT)
							Ball->Position.x += penetration;
						else
							Ball->Position.x -= penetration;
					}
					else
					{
						Ball->Velocity.y = -Ball->Velocity.y;
						float penetration = Ball->Radius - std::abs(diff_vector.y);
						if (dir == UP)
							Ball->Position.y -= penetration;
						else
							Ball->Position.y += penetration;
					}
				}
			}
		}
	}

	for (PowerUp& powerUp : this->PowerUps)
	{
		if (!powerUp.Destroyed)
		{
			if (powerUp.Position.y >= this->Height)
				powerUp.Destroyed = true;
			if (checkCollision(*Player, powerUp))
			{
				ActivatePowerUp(powerUp);
				powerUp.Destroyed = true;
				powerUp.Activated = true;
				SoundEngine->play2D(string(fileP + "/sounds/powerup.wav").c_str(), false);
			}
		}
	}

	Collision result = checkCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
		float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		float percentage = distance / (Player->Size.x / 2.0f);

		float strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		Ball->Stuck = Ball->Sticky;
		SoundEngine->play2D(string(fileP + "/sounds/bleep.wav").c_str(), false);
	}
}

void Game::ResetLevel()
{
	if (this->Level == 0)
		this->Levels[0].Load(string(fileP + "/levels/one.lvl").c_str(), this->Width, this->Height / 2);
	else if (this->Level == 1)
		this->Levels[1].Load(string(fileP + "/levels/two.lvl").c_str(), this->Width, this->Height / 2);
	else if (this->Level == 2)
		this->Levels[2].Load(string(fileP + "/levels/three.lvl").c_str(), this->Width, this->Height / 2);
	else if (this->Level == 3)
		this->Levels[3].Load(string(fileP + "/levels/four.lvl").c_str(), this->Width, this->Height / 2);
	this->Lives = 3;
}

void Game::ResetPlayer()
{
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);

	Effects->Chaos = Effects->Confuse = false;
	Ball->PassThrough = Ball->Sticky = false;
	Player->Color = glm::vec3(1.0f);
	Ball->Color = glm::vec3(1.0f);
}

void Game::SpawnPowerUps(GameObject& block)
{
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("speed", glm::vec3(0.5f, 0.5f, 1.0f), 0.0f, block.Position, ResourceManager::GetTexture("speed")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("sticky", glm::vec3(1.0f, 0.5f, 1.0f), 20.0f, block.Position, ResourceManager::GetTexture("sticky")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("passThrough", glm::vec3(0.5f, 1.0f, 0.5f), 10.0f, block.Position, ResourceManager::GetTexture("passThrough")));
	if (ShouldSpawn(75))
		this->PowerUps.push_back(PowerUp("increase", glm::vec3(1.0f, 0.6f, 0.4f), 0.0f, block.Position, ResourceManager::GetTexture("increase")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("confuse", glm::vec3(1.5f, 0.3f, 0.3f), 15.0f, block.Position, ResourceManager::GetTexture("confuse")));
	if (ShouldSpawn(15))
		this->PowerUps.push_back(PowerUp("chaos", glm::vec3(0.25f, 0.25f, 0.25f), 15.0f, block.Position, ResourceManager::GetTexture("chaos")));

}

void Game::UpdatePowerUps(float dt)
{
	for (PowerUp& powerUp : this->PowerUps)
	{
		powerUp.Position += powerUp.Velocity * dt;
		if (powerUp.Activated)
		{
			powerUp.Duration -= dt;
			if (powerUp.Duration <= 0.0f)
			{
				powerUp.Activated = false;
				if (powerUp.Type == "sticky")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "sticky"))
					{
						Ball->Sticky = false;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "passThrough")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "passThrough"))
					{
						Ball->PassThrough = false;
						Player->Color = glm::vec3(1.0f);
					}
				}
				else if (powerUp.Type == "confuse")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "confuse"))
					{
						Effects->Confuse = false;
					}
				}
				else if (powerUp.Type == "chaos")
				{
					if (!isOtherPowerUpActive(this->PowerUps, "chaos"))
					{
						Effects->Chaos = false;
					}
				}
			}
		}
	}
	this->PowerUps.erase(std::remove_if(this->PowerUps.begin(), this->PowerUps.end(), [](const PowerUp& powerUp)
		{
			return powerUp.Destroyed && !powerUp.Activated;
		}), this->PowerUps.end());
}

// Non-Class methods
bool isOtherPowerUpActive(std::vector<PowerUp>& powerUps, std::string type)
{
	for (const PowerUp& powerUp : powerUps)
	{
		if (powerUp.Activated)
			if (powerUp.Type == type)
				return true;
	}
	return false;
}

void ActivatePowerUp(PowerUp& powerUp)
{
	if (powerUp.Type == "speed")
	{
		Ball->Velocity *= 1.2f;
	}
	else if (powerUp.Type == "sticky")
	{
		Ball->Sticky = true;
		Player->Color = glm::vec3(1.0f, 0.5f, 1.0f);
	}
	else if (powerUp.Type == "passThrough")
	{
		Ball->PassThrough = true;
		Ball->Color = glm::vec3(1.0f, 0.5f, 0.5f);
	}
	else if (powerUp.Type == "increase")
	{
		Player->Size.x += 50;
	}
	else if (powerUp.Type == "confuse")
	{
		if (!Effects->Chaos)
			Effects->Confuse = true;
	}
	else if (powerUp.Type == "chaos")
	{
		if (!Effects->Confuse)
			Effects->Chaos = true;
	}
}

bool ShouldSpawn(unsigned int chance)
{
	unsigned int random = rand() % chance;
	return random == 0;
}

bool checkCollision(GameObject& one, GameObject& two)
{
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x && two.Position.x + two.Size.x >= one.Position.x;
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y && two.Position.y + two.Size.y >= one.Position.y;
	return collisionX && collisionY;
}

Collision checkCollision(BallObject& one, GameObject& two)
{
	glm::vec2 center(one.Position + one.Radius);

	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);

	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);

	glm::vec2 closest = aabb_center + clamped;

	difference = closest - center;

	if (glm::length(difference) <= one.Radius)
		return std::make_tuple(true, vectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction vectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f),
		glm::vec2(1.0f, 0.0f),
		glm::vec2(0.0f, -1.0f),
		glm::vec2(-1.0f, 0.0f)
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}



