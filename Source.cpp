#include <vector>

#include "Shapes2D.h"
#include "Logger.h"
#include "OperationLayer.h"
#include "Globals.h"

#include <time.h>//rand


//class declerations
OpLayer opLayer("test",SCR_WIDTH,SCR_HEIGHT);

float mouseX;
float mouseY;

struct chaser;
std::vector<chaser> chasers;
struct enemyBullet;
std::vector<enemyBullet> enemyBullets;
struct shooter;
std::vector<shooter> shooters;
struct bullet;
std::vector<bullet> bullets;
//0 - gameloop 1 - death state
int gameMode = 1;

float debug_fireRate = 0.07f;
float debug_lastFired = 0.0f;
std::string debug_viewMode[4] = { "standart","colliders","angle lines","full" };
int debug_viewModeSelection = 0;
int debug_bulletCopyCount = 0;
int debug_killCount = 0;

float debug_shipFireRateUpgrades[6] = { 0.01,0.01,0.01,0.003,0.03f,0.03f };
float debug_shipDamageUpgrades[6] = { 0.1,0.1,0.2,0.1,0.1f,0.2f };
int debug_killForUpgrade[6] = { 5,10,25,70 ,150 ,500 };
int debug_upgradeIndex = 0;


// UNUSED first vec: vertices		second: tex coords 
std::pair<std::vector<float>, std::vector<float>> buildGrid()
{
	std::pair<std::vector<float>, std::vector<float>> lines;
	//rows
	for (float i = 1.0f; i < 50.0f; i++)
	{
		Rect temp(Vertex_Type::POSITION_TEXTUREPOS, 50.0f, 0.005f, glm::vec3(0, -5.0f + (i * 0.2f), 0.0f));
		lines.first = global_functions::combineFloatVectors(lines.first, temp.getVertices());
		lines.second = global_functions::combineFloatVectors(lines.second, temp.getTexCoords());
	}
	//cols
	for (float i = 1.0f; i < 50.0f; i++)
	{
		Rect temp(Vertex_Type::POSITION_TEXTUREPOS, 0.005f, 50.0f, glm::vec3(-5.0f + (i * 0.2f),0.0f, 0.0f));
		lines.first = global_functions::combineFloatVectors(lines.first, temp.getVertices());
		lines.second = global_functions::combineFloatVectors(lines.second, temp.getTexCoords());
	}

	return lines;
}

void screenShake()
{
	if (moveOrders.size() ==0)
	{
		moveOrders.push_back({
		mainCam.cameraPos,
		{{0.01,0.01,0.01f},{-0.02,-0.02,-0.02f} ,{0.01,0.01,0.01f} },
		{0.02f,0.02f,0.02f}
			});
	}
}

float playerHp = 150.0f;
glm::vec2 playerCol = {0.12f,0.12f};
glm::vec3 playerPos = glm::vec3(0.0f);
float playerDamage = 2.2f;
struct  bullet
{
	glm::vec3 position;
	//xlengt , ylength
	glm::vec2 collider;
	float speed;
	glm::vec3 velocity = glm::vec3(0.0f,1.0,0.0f);
	float rotationFactor;

	float birth;
	float decayTime = 5.0f;
	bool isDead = false;

	float spreadWidth = 40.0f;
	int randMax = 100;

	float damage = playerDamage;

	glm::mat4 getModel() const
	{
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, position);
		model = glm::rotate(model, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));

		return model;
	}
	void update(float currTime)
	{
		if (currTime - birth > decayTime)
		{
			isDead = true;
			return;
		}
		position += velocity * deltaTime;

	}
	bullet(glm::vec3 pos,glm::vec2 source, glm::vec2 target,float bulSpeed = 1.8f)
	{
		speed = bulSpeed;
		position = pos;
		collider = { 0.025f,0.025f };

		//for some reason there is no need to set seed
		spreadWidth /= 100.0f;
		int temp{ rand() % randMax };
		rotationFactor = global_functions::get_Angle(source, target) - glm::radians(90.0f) ;
		rotationFactor -= glm::radians(randMax * (spreadWidth/2.0f));
		rotationFactor += glm::radians(temp * spreadWidth);

		glm::mat4 velMat = glm::mat4(1.0f);
		velMat = glm::rotate(velMat, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		velocity = velMat *glm::vec4(velocity,1.0f) * speed;

		birth = glfwGetTime();
	}

	//copy constructor
	bullet(const bullet& other)
	{
		//copies all the variables
		memcpy(this, &other, sizeof(bullet));
		std::cout << "A BULLET COPIED " <<std::to_string(debug_bulletCopyCount++) <<"\n";
	}
};
struct  enemyBullet
{
	glm::vec3 position;
	//xlengt , ylength
	glm::vec2 collider;
	float speed;
	glm::vec3 velocity = glm::vec3(0.0f, 1.0, 0.0f);
	float rotationFactor;//in radians

	float birth;
	float decayTime = 12.0f;
	bool isDead = false;

	float damage = 10.0f;

	glm::mat4 getModel() const
	{
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, position);
		model = glm::rotate(model, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::scale(model, { 1.5f, 1.5f, 1.0f });


		return model;
	}
	void update(float currTime)
	{
		if (currTime - birth > decayTime)
		{
			isDead = true;
			return;
		}
		position += velocity * deltaTime;

	}
	enemyBullet(const glm::vec3& pos, const glm::vec3& inVelocity,float rotFac)
	{
		position = pos;
		collider = { 0.05f,0.05f };
		velocity = inVelocity;
		rotationFactor = rotFac;
		birth = glfwGetTime();
	}
};
struct chaser
{
	glm::vec3 position;
	//xlengt , ylength
	glm::vec2 collider;
	float speed;
	glm::vec3 velocity = glm::vec3(0.0f, 1.0, 0.0f);
	float rotationFactor;

	float hp = 30.0f;
	bool isDead = false;

	glm::vec3 debug_lastTargetPos = glm::vec3(0.0f);
	float lastTurnTime = 0.0f;
	glm::mat4 getModel() const
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		model = glm::rotate(model, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		return model;
	}
	void update(float currTime)
	{
		if (hp<0.0f)
			isDead = true;

		if (currTime - lastTurnTime > 1.4f)
		{
			lastTurnTime = currTime;
			debug_lastTargetPos = playerPos;

			velocity = glm::vec3(0.0f, 1.0, 0.0f);
			rotationFactor = global_functions::get_Angle(position,playerPos) + glm::radians(-90.0f);
			glm::mat4 velMat = glm::mat4(1.0f);
			velMat = glm::rotate(velMat, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
			velocity = velMat * glm::vec4(velocity, 1.0f) * speed;
		}
		position += velocity * deltaTime;
	}
	chaser(glm::vec3 pos, glm::vec3 target, float movSpeed= 0.8f)
	{
		position = pos;
		speed = movSpeed;
		collider = { 0.2f,0.2f };
		rotationFactor = global_functions::get_Angle(pos, target) + glm::radians(-90.0f);
		glm::mat4 velMat = glm::mat4(1.0f);
		velMat = glm::rotate(velMat, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		velocity = velMat * glm::vec4(velocity, 1.0f) * speed;

	}
};
struct shooter
{
	glm::vec3 position;
	//xlengt , ylength
	glm::vec2 collider;
	float speed;
	glm::vec3 velocity = glm::vec3(0.0f, 1.0, 0.0f);
	float rotationFactor;

	float hp = 40.0f;
	bool isDead = false;

	glm::vec3 debug_lastTargetPos = glm::vec3(0.0f);
	float lastTurnTime = 0.0f;
	float fireRate = 1.2f;
	float lastFired = 0.0f;
	float bulletSpeed = 1.2f;

	float spreadWidth = 25.0f;
	int randMax = 100;
	glm::mat4 getModel() const
	{
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, position);
		//model = glm::rotate(model, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		return model;
	}

	void fireBullet(const float &currTime)
	{
		if (currTime - lastFired > fireRate)
		{
			lastFired = currTime;

			int randNum{ rand() % randMax };
			float bullRotFactor = global_functions::get_Angle(global_functions::worldTOScreen(position),
				global_functions::worldTOScreen(playerPos)) - glm::radians(90.0f);
			bullRotFactor -= glm::radians(randMax * ((spreadWidth / 100.0f) / 2.0f));
			bullRotFactor += glm::radians(randNum * (spreadWidth / 100.0f));

			glm::mat4 velMat = glm::mat4(1.0f);
			velMat = glm::rotate(velMat, bullRotFactor, glm::vec3(0.0f, 0.0f, 1.0f));
			glm::vec4 bulVelocity = velMat * glm::vec4(0.0f,1.0f,0.0f, 1.0f)*bulletSpeed;

			enemyBullets.emplace_back(position, glm::vec3(bulVelocity.x,bulVelocity.y,bulVelocity.z), bullRotFactor);
		}
	}
	void update(const float& currTime)
	{
		if (hp < 0.0f)
			isDead = true;

		if (currTime - lastTurnTime > 1.5f)
		{
			lastTurnTime = currTime;
			debug_lastTargetPos = playerPos;

			rotationFactor = global_functions::get_Angle(global_functions::worldTOScreen(position),
				global_functions::worldTOScreen(playerPos)) + glm::radians(-90.0f);
			glm::mat4 velMat = glm::mat4(1.0f);
			velMat = glm::rotate(velMat, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
			velocity = velMat * glm::vec4(0.0f, 1.0, 0.0f, 1.0f) * speed;

		}
		position += velocity * deltaTime;
		
		fireBullet(currTime);
	}
	shooter(glm::vec3 pos, glm::vec3 target, float movSpeed = 0.12f)
	{
		position = pos;
		speed = movSpeed;
		collider = { 0.2f,0.2f };

		rotationFactor = global_functions::get_Angle(pos, target) + glm::radians(-90.0f);
		glm::mat4 velMat = glm::mat4(1.0f);
		velMat = glm::rotate(velMat, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));
		velocity = velMat * glm::vec4(velocity, 1.0f) * speed;
	}
};

//****************************************** INPUT
float processInput_camSpeed = 0.03f;
float  processInput_shipSpeed = 0.8f;
void processInput(GLFWwindow* window, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		mainCam.MoveCamera(Camera_Movement::UP, processInput_shipSpeed * deltaTime);
		playerPos.y += processInput_shipSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		mainCam.MoveCamera(Camera_Movement::LEFT, processInput_shipSpeed * deltaTime);
		playerPos.x -= processInput_shipSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		mainCam.MoveCamera(Camera_Movement::DOWN, processInput_shipSpeed * deltaTime);
		playerPos.y -= processInput_shipSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		mainCam.MoveCamera(Camera_Movement::RIGHT, processInput_shipSpeed * deltaTime);
		playerPos.x += processInput_shipSpeed * deltaTime;
	}
	

	//CAMERA
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::RIGHT, processInput_camSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::LEFT, processInput_camSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::DOWN, processInput_camSpeed * deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::UP, processInput_camSpeed * deltaTime);

	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::FORWARD, processInput_camSpeed * deltaTime *2.0f);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		mainCam.ProcessKeyboard(Camera_Movement::BACKWARD, processInput_camSpeed * deltaTime * 2.0f);

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		
		if (glfwGetTime()-debug_lastFired > debug_fireRate)
		{
			bullets.emplace_back(playerPos, global_functions::worldTOScreen(playerPos), glm::vec2(mouseX, mouseY));
			bullets.emplace_back(playerPos, global_functions::worldTOScreen(playerPos), glm::vec2(mouseX, mouseY));
			debug_lastFired = glfwGetTime();
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
	{

		if (glfwGetTime() - debug_lastFired > (double)debug_fireRate+0.3f)
		{
			bullets.emplace_back(playerPos, global_functions::worldTOScreen(playerPos), glm::vec2(mouseX, mouseY));
			debug_lastFired = glfwGetTime();
		}
	}
}
void processInputOverScreen(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
	{
		playerHp = 150.0f;
		gameMode = 0;
	}
}
void keyboard_keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action != GLFW_RELEASE)
		debug_viewModeSelection = 0;
	if (key == GLFW_KEY_2 && action != GLFW_RELEASE)
		debug_viewModeSelection = 1;
	if (key == GLFW_KEY_3 && action != GLFW_RELEASE)
		debug_viewModeSelection = 2;
	if (key == GLFW_KEY_4 && action != GLFW_RELEASE)
		debug_viewModeSelection = 3;
	
}
void mouse_posCallback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = 2.0f / (float)SCR_WIDTH * (float)xpos - 1.0f;
	mouseY = -((2.0f / (float)SCR_HEIGHT) * (float)ypos - 1.0f);

}
void mouse_buttonCallback(GLFWwindow* window,int button,int action,int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action != GLFW_RELEASE)
	{
		moveOrders.push_back({
			mainCam.cameraPos,
			{{-0.02f,0.0f,0.0f},{0.04f,0.0f,0.0f}  ,{-0.02f,0.0f,0.0f}  },
			{0.05f,0.05f,0.05f}
		});
		
	}
}

//****************************************** LOOPS
void lineRenderLoop(const LINE& orthoLine)
{
	//chasers
	for (size_t i = 0; i < chasers.size(); i++)
	{
		glm::vec3 shipScreenCoords = glm::vec3(global_functions::worldTOScreen(chasers[i].debug_lastTargetPos), 0.0f);
		glm::vec3 chaserScreenCoords = glm::vec3(global_functions::worldTOScreen(chasers[i].position), 0.0f);
		opLayer.activeShader->
			setMat4("model",global_functions::get_LineModel(orthoLine.length, chaserScreenCoords, shipScreenCoords));

		opLayer.draw("line");
	}
	//shooters
	for (size_t i = 0; i < shooters.size(); i++)
	{
		glm::vec3 shipScreenCoords = glm::vec3(global_functions::worldTOScreen(shooters[i].debug_lastTargetPos), 0.0f);
		glm::vec3 shooterScreenCoords = glm::vec3(global_functions::worldTOScreen(shooters[i].position), 0.0f);
		opLayer.activeShader->
			setMat4("model", global_functions::get_LineModel(orthoLine.length, shooterScreenCoords, shipScreenCoords));

		opLayer.draw("line");
	}
}
void chaserRenderLoop(const float& currTime = glfwGetTime())
{
	for (size_t i = 0; i < chasers.size(); i++)
	{
		chasers[i].update(currTime);

		//delete and continue if chaser is dead
		if (chasers[i].isDead)
		{
			debug_killCount++;
			chasers.erase(chasers.begin() + i);
			i--;
			continue;
		}

		opLayer.activeShader->setMat4("model", chasers[i].getModel());
		opLayer.draw("ship");
	}

}
void shooterRenderLoop(const float& currTime = glfwGetTime())
{

	for (size_t i = 0; i < shooters.size(); i++)
	{
		shooters[i].update(currTime);

		//delete and continue if chaser is dead
		if (shooters[i].isDead)
		{
			debug_killCount++;
			shooters.erase(shooters.begin() + i);
			i--;
			continue;
		}

		opLayer.activeShader->setMat4("model", shooters[i].getModel());
		opLayer.draw("ship");
	}
}
void bulletRenderLoop(const float &currTime = glfwGetTime())
{
	
	for (size_t i = 0; i < bullets.size(); i++)
	{
		bullets[i].update(currTime);

		if (bullets[i].isDead)
		{
			bullets.erase(bullets.begin() + i);
			i--;
			continue;
		}

		opLayer.activeShader->setMat4("model", bullets[i].getModel());
		opLayer.draw("bullet");
	}
}
void enemyBulletRenderLoop(const float &currTime = glfwGetTime())
{
	
	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		enemyBullets[i].update(currTime);

		if (enemyBullets[i].isDead)
		{
			enemyBullets.erase(enemyBullets.begin() + i);
			i--;
			continue;
		}

		opLayer.activeShader->setMat4("model", enemyBullets[i].getModel());
		opLayer.draw("bullet");
	}
}
void collidersRenderLoop()
{
	glm::mat4 model;
	//bullets
	for (size_t i = 0; i < bullets.size(); i++)
	{
		model = glm::mat4(1.0f);

		model = glm::translate(model,
			bullets[i].position);

		model = glm::scale(model,
			glm::vec3(global_functions::genScaleForColl(bullets[i].collider.x, bullets[i].collider.y, 0.05f, 0.05f), 1.0f)
		);
		opLayer.activeShader->setMat4("model", model);
		opLayer.draw("bullet");
	}
	//enemyBullets
	for (size_t i = 0; i < enemyBullets.size(); i++)
	{
		model = glm::mat4(1.0f);

		model = glm::translate(model,
			enemyBullets[i].position);

		model = glm::scale(model,
			glm::vec3(global_functions::genScaleForColl(enemyBullets[i].collider.x, enemyBullets[i].collider.y, 0.05f, 0.05f), 1.0f)
		);
		opLayer.activeShader->setMat4("model", model);
		opLayer.draw("bullet");

	}
	//chasers
	for (size_t i = 0; i < chasers.size(); i++)
	{
		model = glm::mat4(1.0f);
		
		model = glm::translate(model,
			chasers[i].position);

		model = glm::scale(model,
			glm::vec3(global_functions::genScaleForColl(chasers[i].collider.x, chasers[i].collider.y, 0.2f, 0.2f), 1.0f)
		);
		opLayer.activeShader->setMat4("model", model);
		opLayer.draw("ship");
	}
	//shooter
	for (size_t i = 0; i < shooters.size(); i++)
	{
		model = glm::mat4(1.0f);

		model = glm::translate(model,
			shooters[i].position);

		model = glm::scale(model,
			glm::vec3(global_functions::genScaleForColl(shooters[i].collider.x, shooters[i].collider.y, 0.2f, 0.2f), 1.0f)
		);
		opLayer.activeShader->setMat4("model", model);
		opLayer.draw("ship");
	}
	//player
	model = glm::mat4(1.0f);
	model = glm::translate(model,
		playerPos);
	model = glm::scale(model,
		glm::vec3(global_functions::genScaleForColl(playerCol.x, playerCol.y, 0.2f, 0.2f), 1.0f));
	opLayer.activeShader->setMat4("model", model);
	opLayer.draw("ship");

}

float collisionChecLast = 0.0f;
const float collisionCheckRate= 0.1f;
float enemySpawnLast = 0.0f;
const float enemySpawnRate = 0.2f;
void controlCheck(const float &currentFrame = (float)glfwGetTime())
{
	if (playerHp<0.0f)
	{
		gameMode = 1;
		return;
	}

	//chaser-BUL COLLISION AND BULLET OCCULSION 
	if (currentFrame - collisionChecLast > collisionCheckRate)
	{
		collisionChecLast = currentFrame;
		//bullets loop
		for (size_t i = 0; i < bullets.size(); i++)
		{
			//out of bounds
			if (!bullets[i].isDead && 
				global_functions::isOutOfScreen(bullets[i].position))
			{
				bullets[i].isDead = true;
				continue;
			}
			//chaser-bullet coll
			for (size_t j = 0; j < chasers.size(); j++)
			{
				if (global_functions::checkAABBCollision(chasers[j].collider, chasers[j].position,
					bullets[i].collider, bullets[i].position))
				{
					bullets[i].isDead = true;
					chasers[j].hp -= bullets[i].damage;
				}
			}
			//shooter-bullet coll
			for (size_t j = 0; j < shooters.size(); j++)
			{
				if (global_functions::checkAABBCollision(shooters[j].collider, shooters[j].position,
					bullets[i].collider, bullets[i].position))
				{
					bullets[i].isDead = true;
					shooters[j].hp -= bullets[i].damage;
				}
			}

		
		}//---bullets loop end
		//enemy bullets loop
		for (size_t i = 0; i < enemyBullets.size(); i++)
		{
			//out of bounds
			if (!enemyBullets[i].isDead &&
				global_functions::isOutOfScreen(enemyBullets[i].position))
			{
				enemyBullets[i].isDead = true;
				continue;
			}
			//player-enemyBul coll
			if (global_functions::checkAABBCollision(enemyBullets[i].collider, enemyBullets[i].position,
				playerCol,playerPos))
			{
				screenShake();
				enemyBullets[i].isDead = true;
				playerHp -= enemyBullets[i].damage;
			}
		}//---enemyBullets loop end

		//chaser-PLAYER COLL
		for (size_t i = 0; i < chasers.size(); i++)
		{
			if (!chasers[i].isDead &&
				global_functions::checkAABBCollision(chasers[i].collider, chasers[i].position,
					playerCol, playerPos))
			{
				screenShake();
				playerHp -= 20.0f;
				chasers[i].isDead = true;
			}
		}
		//shooter-PLAYER COLL && TELEPORT FAR SHOOTERS
		for (size_t i = 0; i < shooters.size(); i++)
		{
			if (!shooters[i].isDead &&
				global_functions::checkAABBCollision(shooters[i].collider, shooters[i].position,
					playerCol, playerPos))
			{
				screenShake();
				playerHp -= 25.0f;
				shooters[i].isDead = true;
			}

			//teleport far enemies
			float distanceToPlayer = global_functions::get_Distance(playerPos, shooters[i].position);
			if (distanceToPlayer / 100.0f > 80.0f)
			{
				glm::vec2 teleportCoord;
				teleportCoord.x = ((rand() % 11) + 5) / 10.0f;
				teleportCoord.y = ((rand() % 11) + 5) / 10.0f;
				teleportCoord.x *= (float)glm::pow(-1, rand() % 3);
				teleportCoord.y *= (float)glm::pow(-1, rand() % 5);
				teleportCoord = global_functions::screenTOWorld({ teleportCoord });
				shooters[i].position.x = teleportCoord.x;
				shooters[i].position.y= teleportCoord.y;
			}
		}
	}//bullet collision if end

	//ENEMY SPAWN && UPGRADE CHECK
	if (currentFrame - enemySpawnLast > enemySpawnRate)
	{
		if (debug_killCount >= debug_killForUpgrade[debug_upgradeIndex] &&
			debug_upgradeIndex < 6)
		{
			debug_fireRate -= debug_shipFireRateUpgrades[debug_upgradeIndex];
			playerDamage += debug_shipDamageUpgrades[debug_upgradeIndex];

			debug_shipFireRateUpgrades[debug_upgradeIndex] = 0.0f;
			debug_shipDamageUpgrades[debug_upgradeIndex++] = 0.0f;
		}

		enemySpawnLast = currentFrame;
		float randx = ((rand() % 11) + 10) / 10.0f;
		float randy = ((rand() % 11) + 10) / 10.0f;
		randx *= (float)glm::pow(-1, rand() % 3);
		randy *= (float)glm::pow(-1, rand() % 5);

		chasers.emplace_back(global_functions::screenTOWorld({ randx,randy }), playerPos);
		if (chasers.size() % 2 ==0)
			shooters.emplace_back(global_functions::screenTOWorld({ randx,randy }), playerPos);

	}
}

glm::mat4 playScreen_modelMat;
void loop_playScreen(const float &currentFrame, const LINE &orthoLine)
{
	controlCheck(currentFrame);

	//******************************************	ORTHO
	opLayer.activateShader("ortho");
	//cursor
	playScreen_modelMat = glm::mat4(1.0f);
	playScreen_modelMat = glm::translate(playScreen_modelMat,
		glm::vec3(global_functions::screenTOOrtho(glm::vec2(mouseX, mouseY)), 0.0f));
	opLayer.activeShader->setMat4("model", playScreen_modelMat);
	opLayer.activateTexture("ball");
	opLayer.draw("reticle");
	//ortho lines
	glm::vec3 shipScreenCoords = glm::vec3(global_functions::worldTOScreen(playerPos), 0.0f);
	opLayer.activeShader->setMat4("model", global_functions::get_LineModel(orthoLine.length, { mouseX, mouseY }, shipScreenCoords));
	opLayer.activateTexture("red");
	if (debug_viewModeSelection == 2 || debug_viewModeSelection == 3)
		opLayer.draw("line");
	if (debug_viewModeSelection == 2 || debug_viewModeSelection == 3)
		lineRenderLoop(orthoLine);
	//******************************************	PERSPECTIVE
	opLayer.activateShader("pers");
	//background
	playScreen_modelMat = glm::mat4(1.0f);
	opLayer.activeShader->setMat4("model", playScreen_modelMat);
	opLayer.activateTexture("space");
	opLayer.draw("background");
	//ship
	playScreen_modelMat = glm::mat4(1.0f);
	playScreen_modelMat = glm::translate(playScreen_modelMat, playerPos);
	playScreen_modelMat = glm::rotate(playScreen_modelMat,
		global_functions::get_Angle(shipScreenCoords, glm::vec2(mouseX, mouseY)) - glm::radians(90.f), glm::vec3(0.0f, 0.0f, 1.0f));
	opLayer.activeShader->setMat4("model", playScreen_modelMat);
	opLayer.activeShader->setMat4("view", mainCam.GetViewMatrix());
	opLayer.activateTexture("ship");
	opLayer.draw("ship");

	opLayer.activateTexture("chaser");
	chaserRenderLoop(currentFrame);
	opLayer.activateTexture("shooter");
	shooterRenderLoop(currentFrame);
	opLayer.activateTexture("bullet");
	bulletRenderLoop(currentFrame);
	opLayer.activateTexture("enemyBullet");
	enemyBulletRenderLoop(currentFrame);
	opLayer.activateTexture("red");
	if (debug_viewModeSelection == 1 || debug_viewModeSelection == 3)
		collidersRenderLoop();

	//******************************************	TEXT
	opLayer.activateShader("text");
	opLayer.renderText(
		"kills for next upgrade : " + std::to_string((debug_killForUpgrade[debug_upgradeIndex])) +
		"  kill count : " + std::to_string(debug_killCount) +
		"  view mode : " + debug_viewMode[debug_viewModeSelection],
		15.0f, 45.0f, 0.6f, glm::vec3(0.8, 0.5, 0.0f));
	opLayer.renderText(
		"FPS : " + std::to_string(fps) +
		" playerHp : " + std::to_string(playerHp),
		15.0f, 15.0f, 0.6f, glm::vec3(0.8, 0.5, 0.0f));
	if (debug_viewModeSelection == 3)
		opLayer.renderLogs();
}
glm::mat4 overScreen_modelMat;
void loop_overScreen(const float& currentFrame)
{
	if (bullets.size()>0)
		bullets.clear();
	if (enemyBullets.size() > 0)
		enemyBullets.clear();
	if (chasers.size() > 0)
		chasers.clear();
	if (shooters.size() > 0)
		shooters.clear();
	debug_killCount = 0;
	
	opLayer.activateShader("ortho");

	//cursor
	playScreen_modelMat = glm::mat4(1.0f);
	playScreen_modelMat = glm::translate(playScreen_modelMat,
		glm::vec3(global_functions::screenTOOrtho(glm::vec2(mouseX, mouseY)), 0.0f));
	opLayer.activeShader->setMat4("model", playScreen_modelMat);
	opLayer.activateTexture("ball");
	opLayer.draw("reticle");



	opLayer.activateShader("text");

	opLayer.renderText("GAME OVER",
		0.0f, SCR_HEIGHT / 2.0f, 1.0f, glm::vec3(1.0f,0.0f,0.5f));
	opLayer.renderText("CONTROLS: wasd = movement , left click = fire , RF = camera zoom",
		0.0f, (SCR_HEIGHT / 2.0f) - 100.0f, 0.8f, glm::vec3(0.0f, 0.0f, 1.0f));
	opLayer.renderText("1-5 = debug view modes",
		0.0f, (SCR_HEIGHT / 2.0f) - 130.0f, 0.8f, glm::vec3(0.0f, 0.0f, 1.0f));

	opLayer.renderText("PRESS ENTER TO START",
		0.0f, (SCR_HEIGHT / 2.0f) - 200.0f, 1.0f, glm::vec3(1.0f, 0.0f, 0.5f));
}
int main()
{
	bullets.reserve(2000);
	enemyBullets.reserve(500);
	chasers.reserve(100);
	shooters.reserve(100);

	GLFWwindow* wind = opLayer.get_window();

	glm::mat4 persProjection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, 100.0f);
	glm::mat4  ortProjection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);


	Rect orthoRect(Vertex_Type::POSITION_TEXTUREPOS, 15.0f, 15.0f, 
		glm::vec3((float)SCR_WIDTH / 2.0f,(float)SCR_HEIGHT / 2.0f, 0.0f));
	LINE orthoLine(Vertex_Type::POSITION_TEXTUREPOS,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3( 1.0f, 0.0f , 0.0f));
	Rect shipRect(Vertex_Type::POSITION_TEXTUREPOS, 0.2f, 0.2f);
	Rect bulletRect(Vertex_Type::POSITION_TEXTUREPOS, 0.05f, 0.05f);
	Rect backgroundRect(Vertex_Type::POSITION_TEXTUREPOS, 100.0f, 100.0f, {0.0f,0.0f,-1.0f});

	glfwSetMouseButtonCallback(wind, mouse_buttonCallback);
	glfwSetCursorPosCallback(wind, mouse_posCallback);
	glfwSetKeyCallback(wind, keyboard_keyCallback);
	glfwSetInputMode(wind, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// ****************************************** ADDING STUFF
	opLayer.resourceManager.add_shaderProgram("main", "shaders/vertex.glsl", "shaders/fragment.glsl");
	opLayer.resourceManager.add_shaderProgram("text", "shaders/vertexText.glsl", "shaders/fragmentText.glsl");
	opLayer.resourceManager.add_shaderProgram("ortho", "shaders/vertexOrtho.glsl", "shaders/fragmentOrtho.glsl");

	opLayer.resourceManager.add_bufferState("main", Buffer_Type::CONSECUTIVE, Vertex_Type::POSITION_TEXTUREPOS);
	opLayer.resourceManager.add_texture("ball", "resources/pong_ball.png");
	opLayer.resourceManager.add_texture("red", "resources/collider2.png");
	opLayer.resourceManager.add_texture("ship", "resources/ship.png");
	opLayer.resourceManager.add_texture("bullet", "resources/bullet2.png");
	opLayer.resourceManager.add_texture("shooter", "resources/enemyship_red.png");
	opLayer.resourceManager.add_texture("enemyBullet", "resources/bullet_red2.png");
	opLayer.resourceManager.add_texture("chaser", "resources/ship_purple.png");
	opLayer.resourceManager.add_texture("space", "resources/2k_stars.jpg");

	opLayer.resourceManager.add_bufferSegment("reticle", Draw_Method::TRIANGLES, orthoRect.getVertices(), orthoRect.getTexCoords());
	opLayer.resourceManager.add_bufferSegment("ship", Draw_Method::TRIANGLES, shipRect.getVertices(), shipRect.getTexCoords());
	opLayer.resourceManager.add_bufferSegment("line", Draw_Method::LINES, orthoLine.getVertices(), orthoLine.getTexCoords());
	opLayer.resourceManager.add_bufferSegment("bullet", Draw_Method::TRIANGLES, bulletRect.getVertices(), bulletRect.getTexCoords());
	opLayer.resourceManager.add_bufferSegment("background", Draw_Method::TRIANGLES, backgroundRect.getVertices(), backgroundRect.getTexCoords());

	// ****************************************** SHADER CONFIG
	opLayer.activateShader("pers");
	opLayer.activeShader->setMat4("projection", persProjection);
	glm::mat4 debugModel = glm::mat4(1.0f);
	opLayer.activeShader->setMat4("model", debugModel);
	opLayer.activateShader("text");
	opLayer.activeShader->setMat4("projection", ortProjection);
	opLayer.activateShader("ortho");
	opLayer.activeShader->setMat4("projection", ortProjection);

	opLayer.bindBufferState("main");

	//var definitions for loop
	float currentFrame;
	glm::mat4 modelMat;
	while (!glfwWindowShouldClose(wind))
	{
		//delta time 
		currentFrame = glfwGetTime();
		global_functions::loop_calcDeltaTime(currentFrame);
		//fps
		global_functions::loop_calcFps(currentFrame);
		global_functions::loop_updatMoveOrders(currentFrame);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (gameMode==0)
		{
			processInput(wind, deltaTime);
			loop_playScreen(currentFrame, orthoLine);
		}
		if (gameMode == 1)
		{
			processInputOverScreen(wind);
			loop_overScreen(currentFrame);
		}

		glfwSwapBuffers(wind);
		glfwPollEvents();
	}
	glfwTerminate();
}