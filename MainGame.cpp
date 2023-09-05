#define PLAY_IMPLEMENTATION
#define PLAY_USING_GAMEOBJECT_MANAGER
#include "Play.h"

//Sets the canvas size for the game

int DISPLAY_WIDTH = 1280;
int DISPLAY_HEIGHT = 720;
int DISPLAY_SCALE = 1;

enum Agent8State
{
	STATE_APPEAR = 0,
	STATE_HALT,
	STATE_PLAY,
	STATE_DEAD,
};

struct GameState // Initialises variables for the game
{
	int score = 0;
	Agent8State agentState = STATE_APPEAR;
};

GameState gameState;

enum GameObjectType // Creates enumerables of the various object types in the game
{
	TYPE_NULL = -1,
	TYPE_AGENT8,
	TYPE_FAN,
	TYPE_TOOL,
	TYPE_COIN,
	TYPE_STAR,
	TYPE_LASER,
	TYPE_DESTROYED,
};

// Function Declarations to be used later in the code
void HandlePlayerControls();
void UpdateFan();
void UpdateTools();
void UpdateCoinsAndStars();
void UpdateLaser();
void UpdateDestroyed();
void UpdateAgent8();

// The entry point for a PlayBuffer program
void MainGameEntry( PLAY_IGNORE_COMMAND_LINE )
{
	Play::CreateManager( DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_SCALE ); //Creates the window according to the variables specified at the start of the code
	Play::CentreAllSpriteOrigins(); // Makes sure the origin of all sprites are within the center of their image
	Play::LoadBackground("Data\\Backgrounds\\background.png"); // Sets the background of the game to match the file directory
	Play::StartAudioLoop("music"); // Finds the first audio file with 'music' in its name within the 'Audio' file
	Play::CreateGameObject(TYPE_AGENT8, { 115, 0 }, 50, "agent8"); // Creates the origin of the Agent8 Sprite with the position, collision size and the sprite name & draws it as the game starts

	int id_fan = Play::CreateGameObject(TYPE_FAN, { 1140, 217 }, 0, "fan"); // Creates the origin of the fan sprite and draws it
	Play::GetGameObject (id_fan).velocity = { 0, 3 }; // Sets the variables for the object fan so it can animate instantly
	Play::GetGameObject (id_fan).animSpeed = 1.0f;
}

// Called by PlayBuffer every frame (60 times a second!)
bool MainGameUpdate( float elapsedTime )
{
	Play::DrawBackground();// Draws the background of the game

	// Calls the pre-declared functions
	UpdateAgent8();
	HandlePlayerControls();
	UpdateFan();
	UpdateTools();
	UpdateCoinsAndStars();
	UpdateLaser();
	UpdateDestroyed();

	// Draws the text for instructions and the current score
	Play::DrawFontText("64px", "ARROW KEYS TO MOVE UP AND DOWN AND SPACE TO FIRE",
		{DISPLAY_WIDTH / 2, DISPLAY_HEIGHT - 30}, Play::CENTRE);
	Play::DrawFontText("132px", "SCORE: " + std::to_string(gameState.score),
		{ DISPLAY_WIDTH / 2, 50 }, Play::CENTRE);

	// Updates the canvas with the sprite changes waiting to be presented
	Play::PresentDrawingBuffer(); 

	// Exits the game if the ESC key is pressed
	return Play::KeyDown( VK_ESCAPE );
}

// Function handling all the controls the player has
void HandlePlayerControls()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType (TYPE_AGENT8); // Obtains a reference to the object the player is controlling

	// Causes Agent 8 to move upwards & change their sprite accordingly if the UP arrow key is pressed
	if (Play::KeyDown(VK_UP))
	{
		obj_agent8.velocity = { 0, -4 };
		Play::SetSprite(obj_agent8, "agent8_climb", 0.25f);
	}

	// Causes Agent 8 to move downwards if the DOWN arrow key is pressed
	else if (Play::KeyDown(VK_DOWN))
	{
		obj_agent8.acceleration = { 0, 0.15f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
	}


	else
	{

		// If player's velocity is lower than 5, sets the state to halt
		if (obj_agent8.velocity.y > 5)
		{
			gameState.agentState = STATE_HALT;
			Play::SetSprite(obj_agent8, "agent8_halt", 0.333f);
			obj_agent8.acceleration = { 0, 0 };
		}

		// If no key is pressed, slow down the velocity of Agent 8 and cause them to stay still
		else
		{
			Play::SetSprite(obj_agent8, "agent8_hang", 0.02f);
			obj_agent8.velocity *= 0.5f;
			obj_agent8.acceleration = { 0, 0 };
		}
	}


	// If space is pressed, creates and fires a laser sprite which moves across the screen going from the player's gun to the right
	if (Play::KeyPressed(VK_SPACE))
	{
		Vector2D firePos = obj_agent8.pos + Vector2D(155, -75);
		int id = Play::CreateGameObject(TYPE_LASER, firePos, 30, "laser");
		Play::GetGameObject(id).velocity = { 32, 0 };
		Play::PlayAudio("shoot");
	}

	Play::UpdateGameObject(obj_agent8); // Updates all the location & movement data of Agent 8

	if (Play::IsLeavingDisplayArea(obj_agent8)) // Checks if the player is trying to move out of bounds and prevents it.
	{
		obj_agent8.pos = obj_agent8.oldPos; // Sets the position of the player back to the original placement to avoid moving out of bounds
	}

	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite); // Draws the web line from the Agent 8 sprite to the top of the screen
	Play::DrawObjectRotated(obj_agent8); // Applies the updated location data to the Agent 8 sprite & draws it.
}

// Function to control the fan object
void UpdateFan() 
{
	GameObject& obj_fan = Play::GetGameObjectByType(TYPE_FAN); // Obtains a reference

	// If a 50/50 is rolled, spawns a screwdriver sprite at the fan at a random velocity
	if (Play::RandomRoll(50) == 50)
	{
		int id = Play::CreateGameObject(TYPE_TOOL, obj_fan.pos, 50, "driver");
		GameObject& obj_tool = Play::GetGameObject(id);
		obj_tool.velocity = Point2f(-8, Play::RandomRollRange(-1, 1) * 6);

		// 50% of the time it changes to be a spanner with a different speed, velocity, size and rotation speed
		if (Play::RandomRoll(2) == 1)
		{
			Play::SetSprite(obj_tool, "spanner", 0);
			obj_tool.radius = 100;
			obj_tool.velocity.x = -4;
			obj_tool.rotSpeed = 0.1f;
		}
		Play::PlayAudio("tool");
	}

	// 1/150 chance to spawn a coin instead
	if (Play::RandomRoll(150) == 1)
	{
		int id = Play::CreateGameObject(TYPE_COIN, obj_fan.pos, 40, "coin");
		GameObject& obj_coin = Play::GetGameObject(id);
		obj_coin.velocity = { -3, 0 };
		obj_coin.rotSpeed = 0.1f;
	}

		Play::UpdateGameObject (obj_fan);

		//If the fan moves out of the boundaries, make it change direction instead
		if (Play::IsLeavingDisplayArea(obj_fan))
		{
			obj_fan.pos = obj_fan.oldPos;
			obj_fan.velocity.y *= -1;
		}
	Play::DrawObject(obj_fan); // Draws the fan onto the canvas
}

// Function controls the tool objects
void UpdateTools()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Obtains a reference to the player character to aid with collisions
	std::vector <int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL); //Finds all objects under the 'Tool' type and stores them into the local vector

	for (int id : vTools)
	{
		GameObject& obj_tool = Play::GetGameObject(id); // Assigns each vector a unique ID to use as a reference

		// If the object collides with Agent 8, plays the death music and removes them from the screen.
		if (gameState.agentState != STATE_DEAD && Play::IsColliding(obj_tool, obj_agent8))
		{
		Play::StopAudioLoop("music");
		Play::PlayAudio("die");
		gameState.agentState = STATE_DEAD;
		}

		Play::UpdateGameObject(obj_tool); // Updates the positioning of the tool objects


		//If the objects hit the top and bottom boundaries, the velocity reverts and they bounce
		if (Play::IsLeavingDisplayArea(obj_tool, Play::VERTICAL))
		{
			obj_tool.pos = obj_tool.oldPos;
			obj_tool.velocity.y *= -1;
		}

		Play::DrawObjectRotated(obj_tool); // Draws the objects

		// If the object isn't visible anymore, remove from the game
		if (!Play::IsVisible(obj_tool))
		{
			Play::DestroyGameObject(id);
		}
	}
}

// Function controls the coins and stars
void UpdateCoinsAndStars()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Grabs reference of Agent 8 for collecting coins
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN); //Creates coins to a localised vector

	for (int id_coin : vCoins)
	{
		GameObject& obj_coin = Play::GetGameObject(id_coin); // Gets the reference of the coin object
		bool hasCollided = false; // Sets the collision with player to be false

		if (Play::IsColliding(obj_coin, obj_agent8)) // Checks for if the player collides with a coin
		{
			for (float rad{ 0.25f }; rad < 2.0f; rad += 0.5f) // Creates stars that spawn from the coin in each intercardinal direction
			{
				int id = Play::CreateGameObject(TYPE_STAR, obj_agent8.pos, 0, "star");
				GameObject& obj_star = Play::GetGameObject(id);
				obj_star.rotSpeed = 0.1f;
				obj_star.acceleration = { 0.0f, 0.5f };
				Play::SetGameObjectDirection(obj_star, 16, rad * PLAY_PI);
			}

			// if a collision has happened then increase the point score
			hasCollided = true;
			gameState.score += 500;
			Play::PlayAudio("collect");
		}
		// Updates the variables and draws the new position and rotation
		Play::UpdateGameObject(obj_coin);
		Play::DrawObjectRotated(obj_coin);

		// Deletes the object once the coin is invisible or has collided with the player
		if (!Play::IsVisible(obj_coin) || hasCollided)
		{
			Play::DestroyGameObject(id_coin);
		}

		std::vector<int> vStars = Play::CollectGameObjectIDsByType(TYPE_STAR); // Grabs the star objects and places into a vector

		// Moves and deletes the stars once they move out of vision
		for (int id_star : vStars)
		{
			GameObject& obj_star = Play::GetGameObject(id_star);

			Play::UpdateGameObject(obj_star);
			Play::DrawObjectRotated(obj_star);

			if (!Play::IsVisible(obj_star))
			{
				Play::DestroyGameObject(id_star);
			}
		}
	}
}

// Function controls the lasers
void UpdateLaser()
{
	// Grabs the information of lasers, tools and coins into seperate vectors to be used within the function
	std::vector<int> vLasers = Play::CollectGameObjectIDsByType(TYPE_LASER);
	std::vector<int> vTools = Play::CollectGameObjectIDsByType(TYPE_TOOL);
	std::vector<int> vCoins = Play::CollectGameObjectIDsByType(TYPE_COIN);

	// for every laser within the vector laser:
	for (int id_laser : vLasers)
	{
		GameObject& obj_laser = Play::GetGameObject(id_laser); // Grabs references of laser object, and sets the default collision state to false
		bool hasCollided = false;
		for (int id_tool : vTools)
		{
			GameObject& obj_tool = Play::GetGameObject(id_tool);
			if (Play::IsColliding(obj_laser, obj_tool)) // For every tool within tool, checks to see if they have collided with the laser
			{
				hasCollided = true; // Sets the laser collision value to be true
				obj_tool.type = TYPE_DESTROYED; // Sets the type of tool to be destroyed
				gameState.score += 100; // Increments the score by 100
			}
		}

		// Same with the tools but with coins
		for (int id_coin : vCoins)
		{
			GameObject& obj_coin = Play::GetGameObject(id_coin);
			if (Play::IsColliding(obj_laser, obj_coin))
			{
				hasCollided = true;
				obj_coin.type = TYPE_DESTROYED;
				Play::PlayAudio("error");
				gameState.score -= 300; // Decrements the score by 300 if you destroy a coin
			}
		}

		// Score cannot go below 0
		if (gameState.score < 0)
		{
			gameState.score = 0;
		}

		// Updates the values and redraws the objects
		Play::UpdateGameObject(obj_laser);
		Play::DrawObject(obj_laser);

		// If the laser goes offscreen or collides with an object, destroy it
		if (!Play::IsVisible(obj_laser) || hasCollided)
		{
			Play::DestroyGameObject(id_laser);
		}
	}

 }

// Function controls destroyed objects
void UpdateDestroyed()
{
	std::vector<int> vDead = Play::CollectGameObjectIDsByType(TYPE_DESTROYED);

	for (int id_dead : vDead)
	{
		GameObject& obj_dead = Play::GetGameObject(id_dead);
		obj_dead.animSpeed = 0.2f; // Once the object is considered destroyed, sets the animation speed
		Play::UpdateGameObject(obj_dead); // Updates the object to be dead

		// Causes the objects to still move, but cannot be interacted with
		if (obj_dead.frame % 2)
		{
			Play::DrawObjectRotated(obj_dead, (10 - obj_dead.frame) / 10.0f);

			// Once 10 animation frames has passed, destroys the object
			if (!Play::IsVisible(obj_dead) || obj_dead.frame >= 10)
			{
				Play::DestroyGameObject(id_dead);
			}
		}
	}
}

// Function controls the player states
void UpdateAgent8()
{
	GameObject& obj_agent8 = Play::GetGameObjectByType(TYPE_AGENT8); // Gets a reference for the different states for the player

	switch (gameState.agentState)
	{
	
	// Game start makes the player move 1/3 of the screen downwards from the top before changing to play state
	case STATE_APPEAR:
		obj_agent8.velocity = { 0, 12 };
		obj_agent8.acceleration = { 0, 0.5f };
		Play::SetSprite(obj_agent8, "agent8_fall", 0);
		obj_agent8.rotation = 0;
		if (obj_agent8.pos.y >= DISPLAY_HEIGHT / 3)
		{
			gameState.agentState = STATE_PLAY;
		}
		break;


	// Lowers the velocity whilst there is no input from the player until the animation is complete, then changes to play state
	case STATE_HALT:
		obj_agent8.velocity *= 0.9f;
		if (Play::IsAnimationComplete(obj_agent8))
		{
			gameState.agentState = STATE_PLAY;
		}
		break;


	// Play state gives control to the player
	case STATE_PLAY:
		HandlePlayerControls();
		break;


	// Dead state causes agent 8 to spin and move offscreen slowly until space is pressed, then sets the game state to appear and restarts the game,
	// destroying current objects to avoid dying on spawn
	case STATE_DEAD:
		obj_agent8.acceleration = { -0.3f, 0.5f };
		obj_agent8.rotation += 0.25f;
		if (Play::KeyPressed(VK_SPACE) == true)
		{
			gameState.agentState = STATE_APPEAR;
			obj_agent8.pos = { 115, 0 };
			obj_agent8.velocity = { 0, 0 };
			obj_agent8.frame = 0;
			Play::StartAudioLoop("music");
			gameState.score = 0;

			for (int id_obj : Play::CollectGameObjectIDsByType(TYPE_TOOL))
			{
				Play::GetGameObject(id_obj).type = TYPE_DESTROYED;
			}

			for (int id_coin : Play::CollectGameObjectIDsByType(TYPE_COIN))
			{
				Play::GetGameObject(id_coin).type = TYPE_DESTROYED;
			}
		}
		break;

	} // End of switch of Agent8State


	// Updates the position of the player
	Play::UpdateGameObject(obj_agent8);

	//If the player tries to leave the display area whilst not being in the dead state, prevents it
	if (Play::IsLeavingDisplayArea(obj_agent8) && gameState.agentState != STATE_DEAD)
	{
		obj_agent8.pos = obj_agent8.oldPos;
	}

	// Draws the white web line from agent 8 to the top of the screen
	Play::DrawLine({ obj_agent8.pos.x, 0 }, obj_agent8.pos, Play::cWhite);

	//Draws agent 8 with any new changes to the position
	Play::DrawObjectRotated(obj_agent8);
}

// Gets called once when the player quits the game 
int MainGameExit( void )
{
	Play::DestroyManager();
	return PLAY_OK;
}

