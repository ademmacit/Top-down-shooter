#ifndef GLOBALS_H
#define GLOBALS_H


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm> //std::min

#include "Camera.h"
#include "Logger.h"


struct moveOrder;
//***************************** VARIABLES

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const  float ASPECT_X = 1.6f;
const  float ASPECT_Y = 0.9f;

const float WORLD_NEAR = 0.001f;
const float WORLD_FAR = 100.0f;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
int frameCounter = 0;
int fps = 0;
float lastFpsUpdateTime = 0.0f;

std::vector<moveOrder> moveOrders;
float moveOrderFreq = 0.02f;
float lastMoveOrderCheck = 0.0f;

Camera mainCam(glm::vec3(0.0f,0.0f,2.0f));
Logger logger;

//***************************** CLASSES

/*
**changes given vec3 ref gradually based on timeWindow.
**at the end of the timeWindow given given movement is added to ref.
**additions happen when update function is called.
**if given a vector of timewindows and movements executes them back to back
*DONT create movOrders for deletable vecs. if vec is deleted update call on the order returns an error
*/
struct moveOrder
{
	glm::vec3& pos;
	std::vector<glm::vec3> movements;
	std::vector<float> timeWindows;
	float defineTime;
	bool isDead = false;

	//holds the changes done in last update
	glm::vec3 changeDone = glm::vec3(0.0f);

	void update(const float& currTime)
	{
		//if delay hasnt ended dont do anything
		if (defineTime > currTime || isDead)
			return;

		float percnetage;
		if (currTime - defineTime >= timeWindows[0])
			percnetage = 1.0f;
		else
			percnetage = (currTime - defineTime) / timeWindows[0];

		//by substacting changes done by this movement we get current pos value
		pos = (pos - changeDone) + (movements[0] * percnetage);
		changeDone = movements[0] * percnetage;

		if (percnetage >= 1.0f)
		{
			movements.erase(movements.begin(), movements.begin() + 1);
			timeWindows.erase(timeWindows.begin(), timeWindows.begin() + 1);
			defineTime = currTime;
			changeDone = { 0.0f,0.0f,0.0f };
			if (timeWindows.size() == 0)
				isDead = true;
		}
	}
	moveOrder(glm::vec3& pos, glm::vec3 movement, float timeWindow, float startDelay = 0.0f) :
		pos(pos)
	{
		movements.push_back(movement);
		timeWindows.push_back(timeWindow);
		defineTime = (float)glfwGetTime() + startDelay;
	}
	moveOrder(glm::vec3& pos, std::vector<glm::vec3> movements, std::vector<float> timeWindows, float startDelay = 0.0f) :
		pos(pos), movements(movements), timeWindows(timeWindows)
	{

		defineTime = (float)glfwGetTime() + startDelay;
	}

	//placeholder overload. change in future
	moveOrder operator=(const moveOrder& other)
	{
		return *this;
	}
};
template <typename T>
class CircQue
{
public:
	CircQue(int size, T DefaultNullValue);
	void enque(T itemToAdd)
	{
		if (
			(head == 0 && tail == size - 1) ||
			(tail == (head - 1) % (size - 1))
			)
		{
			//std::cout << "FULL   " << asd << std::endl;
			return;
		}

		else if (head == -1) //first element
		{
			tail = head = 0;
			arr[tail] = itemToAdd;
		}
		else if (tail == size - 1 && head != 0) //loop
		{
			tail = 0;
			arr[tail] = itemToAdd;
		}
		else
		{
			tail++;
			arr[tail] = itemToAdd;
		}
	}
	T deque()
	{
		if (head == -1)
		{
			//std::cout << "EMPTY   " << std::endl;
			return nullVal;
		}

		T temp = arr[head];


		if (head == tail)
			head = tail = -1;
		else if (head == size - 1)
			head = 0;
		else
			head++;

		return temp;

	}

	bool isEmpty() { if (head == -1) { return true; } else { return false; } }

	void writeToConsole()
	{
		if (head == -1)
		{
			std::cout << "que is empty" << std::endl;
			return;
		}

		if (tail >= head)
		{
			for (size_t i = head; i <= tail; i++)
				std::cout << arr[i].first << " " << arr[i].second << std::endl;
		}
		else
		{
			for (size_t i = head; i < size; i++)
				std::cout << arr[i].first << " " << arr[i].second << std::endl;
			for (size_t i = 0; i <= tail; i++)
				std::cout << arr[i].first << " " << arr[i].second << std::endl;
		}

	}


	T* arr;
	int head;
	int tail;
	int size;
private:
	T nullVal;

};
template <typename T>
CircQue<T>::CircQue(int sizeForQue, T DefaultNullValue)
{
	nullVal = DefaultNullValue;
	size = sizeForQue;
	arr = new T[sizeForQue];
	head = -1;
	tail = -1;
}



//***************************** FUNCTIONS

namespace global_functions
{
	//******************************* MAIN
	void loop_updatMoveOrders(const float& currentFrame = (float)glfwGetTime())
	{
		if (currentFrame - lastMoveOrderCheck >= moveOrderFreq)
		{
			lastMoveOrderCheck = currentFrame;
			for (size_t i = 0; i < moveOrders.size(); i++)
			{
				if (moveOrders[i].isDead)
					moveOrders.erase(moveOrders.begin(), moveOrders.begin() + 1);
				else
					moveOrders[i].update(currentFrame);
			}
		}

		
	}
	void loop_calcDeltaTime(const float &currentFrame = (float)glfwGetTime())
	{
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}
	void loop_calcFps(const float &currentFrame = (float)glfwGetTime())
	{
		if (currentFrame - lastFpsUpdateTime >= 1.0f)
		{
			fps = frameCounter;
			frameCounter = 0;
			lastFpsUpdateTime = currentFrame;
		}
		else
			frameCounter++;
	}

	bool writeToTextFile(std::string filePath, std::string text)
	{
		std::ofstream myfile;
		myfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			myfile.open(filePath);
			myfile << text << std::endl;
			myfile.close();
			return true;
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "error while writing to file" << std::endl;
			return false;
		}
	}
	bool appendToTextFile(std::string filePath, std::string text)
	{
		std::ofstream myfile;
		myfile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			myfile.open(filePath, std::ios::app);
			myfile << text << std::endl;
			myfile.close();
			return true;
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "error while writing to file" << std::endl;
			return false;
		}
	}
	std::string readFromTextFile(std::string filePath)
	{
		std::ifstream myFile;
		std::string text;
		myFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try
		{
			// open files
			myFile.open(filePath);
			std::stringstream sstream;
			// read file's buffer contents into streams
			sstream << myFile.rdbuf();
			// close file handlers
			myFile.close();
			// convert stream into string
			text = sstream.str();
			return text;
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "error while reading from file" << std::endl;
			return "error while reading from file";
		}

	}
	std::vector<float> combineFloatVectors(std::vector<float> fVec1, std::vector<float> fVec2)
	{
		std::vector<float> Sum;
		Sum.insert(Sum.end(), fVec1.begin(), fVec1.end());
		Sum.insert(Sum.end(), fVec2.begin(), fVec2.end());
		return Sum;
	}
	/*
	**splits the txt string according to targetCh and sets the result parts in strs
	*1st param=string	2nd=return vector	3rd=target char 
	*returns the size of strs
	*/
	size_t splitString(const std::string& txt, std::vector<std::string>& strs, char targetCh)
	{
		size_t pos = txt.find(targetCh);
		size_t initialPos = 0;
		strs.clear();

		//decompose string || std::string::npos is the end of the string so while loops all of the string
		//strs.push_back(txt.substr(initialPos, pos - initialPos)); || pushes the substring between initialpos and pos
		//
		while (pos != std::string::npos)
		{
			strs.push_back(txt.substr(initialPos, pos - initialPos));
			initialPos = pos + 1;

			pos = txt.find(targetCh, initialPos);
		}
		//add the last one
		strs.push_back(txt.substr(initialPos, std::min(pos, txt.size()) - initialPos + 1));

		return strs.size();
	}

	/*
	**linear interpolation: returns the point between A and A vecs based on t
	**example: A=0,0,0,0	B=2,0,0,0	t=0.5	return=1,0,0,0
	*t can be between 0 and 1 
	*if t = 0 returns second vec  
	*if t = 1 returns first vec
	*/
	static glm::vec4 Mylerp(const glm::vec4& A, const glm::vec4& B, float t) {
		return A * t + B * (1.f - t);
	}
	void snapToGrid(glm::vec3& coord, float x_interval, float y_interval)
	{

		if (glm::abs(fmod(coord.x, x_interval)) >= x_interval / 2.0f)
			if (coord.x <= 0.0f)        // -       -0.36f = -0.16f 0.4    -=    
				coord.x -= x_interval + fmod(coord.x, x_interval);
			else                            // +		0.36  =  0.16f 0.4    +=
				coord.x += x_interval - fmod(coord.x, x_interval);
		else
			coord.x -= fmod(coord.x, x_interval);


		if (glm::abs(fmod(coord.y, y_interval)) >= y_interval / 2.0f)
			if (coord.y <= 0.0f)
				coord.y -= y_interval + fmod(coord.y, y_interval);
			else
				coord.y += y_interval - fmod(coord.y, y_interval);
		else
			coord.y -= fmod(coord.y, y_interval);
	}
	bool checkAABBCollision(const glm::vec2& box1Coll,const glm::vec3& box1Pos,const glm::vec2& box2Coll,const glm::vec3& box2Pos)
	{
		bool xOverlap = false, yOverlap = false;

		if (box1Pos.x + (box1Coll.x / 2.0f) >= box2Pos.x - (box2Coll.x / 2.0f))
		{
			if (box1Pos.x - (box1Coll.x / 2.0f) <= box2Pos.x + (box2Coll.x / 2.0f))
				xOverlap = true;
		}
		if (box1Pos.y + (box1Coll.y / 2.0f) >= box2Pos.y - (box2Coll.y / 2.0f))
		{
			if (box1Pos.y - (box1Coll.y / 2.0f) <= box2Pos.y + (box2Coll.y / 2.0f))
				yOverlap = true;
		}

		if (xOverlap && yOverlap)
			return true;
		else
			return false;
	}

	glm::vec3 screenTOWorld(const glm::vec2& coords, float targetPlaneZ = 0.0f)
	{
		glm::mat4 persProjection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, WORLD_NEAR, WORLD_FAR);

		glm::mat4 mvp = persProjection * mainCam.GetViewMatrix();
		glm::mat4 inverseMvp = glm::inverse(mvp);

		glm::vec4 nearCursor = glm::vec4(coords.x, coords.y, -1.0f, 1.0f);
		glm::vec4 farCursor = glm::vec4(coords.x, coords.y, 1.0f, 1.0f);

		glm::vec4 nearWorldCursor = inverseMvp * nearCursor;
		glm::vec4 farWorldCursor = inverseMvp * farCursor;

		nearWorldCursor = nearWorldCursor * (1.0f / nearWorldCursor.w);
		farWorldCursor = farWorldCursor * (1.0f / farWorldCursor.w);

		//normalizing camera.z to be between 0-1 for lerp
		//if desired world plane is differnt than z=0 add it like .. - (maincam.cameaPos.z + desiredZ) - ..
		float t = 1.0f - (mainCam.cameraPos.z + targetPlaneZ - WORLD_NEAR) / (WORLD_FAR - WORLD_NEAR);

		glm::vec4 after = global_functions::Mylerp(nearWorldCursor, farWorldCursor, t);

		return glm::vec3(after.x, after.y, after.z);
	}
	glm::vec2 worldTOScreen(const glm::vec3& coords)
	{
		glm::mat4 persProjection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, WORLD_NEAR, WORLD_FAR);
		glm::mat4 mvp = persProjection * mainCam.GetViewMatrix();
		glm::vec4 after = mvp * glm::vec4(coords, 1.0f);
		after = after * (1.0f / mainCam.cameraPos.z);

		return glm::vec2(after.x, after.y);
	}
	glm::vec2 screenTOOrtho(const glm::vec2& coords)
	{
		glm::vec2 tempVal = coords;
		//screen coords are in (-1,1) screen coords so by multiplying we make it ready to ortho
		tempVal.x *= (float)SCR_WIDTH / 2.0f;
		tempVal.y *= (float)SCR_HEIGHT / 2.0f;
		return tempVal;
	}
	/*
	**returns the angle between 2 points in radians
	*for some reason its accurate in screencoords
	*/
	float get_Angle(const glm::vec2& P1, const glm::vec2& P2)
	{
		float angle = glm::acos(glm::dot(glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)),
			glm::normalize(glm::vec3((P2.x - P1.x) * ASPECT_X, (P2.y - P1.y) * ASPECT_Y, 0.0f))));

		if (P2.y < P1.y)
			return (glm::radians(180.0f) - angle) + glm::radians(180.0f);
		else
			return angle;

		return angle;
	}
	/*
	**checks the givem coords if out of view by converting it to screen coords
	*/
	bool isOutOfScreen(const glm::vec3& pos)
	{
		glm::vec2 screenPos = global_functions::worldTOScreen(pos);
		if (screenPos.x < -1.0f || screenPos.x > 1.0f ||
			screenPos.y < -1.0f || screenPos.y > 1.0f)
		{
			return true;
		}
		return false;
	}
	/*
	**returns model to draw a ortho line between 2 points
	*Points are on screen coords
	*/
	glm::mat4 get_LineModel(const float& lineLength, const glm::vec2& P1, const glm::vec2& P2)
	{
		float halfWidth = SCR_WIDTH / 2.0f;
		float halfHeight = SCR_HEIGHT / 2.0f;

		glm::mat4 model = glm::mat4(1.0f);

		float sourceToMouseDist = glm::sqrt(
			((halfWidth * glm::abs(P2.x - P1.x)) * (halfWidth * glm::abs(P2.x - P1.x))) +
			((halfHeight * glm::abs(P2.y - P1.y)) * (halfHeight * glm::abs(P2.y - P1.y))));

		//divide with linelength to find accurate scale even if linelength is different than 1
		float orgLineLengt = glm::abs(lineLength);
		glm::vec3 scaleFactor = glm::vec3(sourceToMouseDist / orgLineLengt, 1.0f, 1.0f);

		float rotationFactor = global_functions::get_Angle(P1, P2);

		//bring to center
		model = glm::translate(model, glm::vec3(halfWidth + P1.x * halfWidth, halfHeight + P1.y * halfHeight, 0.0f));
		//rotate for mouse pos
		model = glm::rotate(model, rotationFactor, glm::vec3(0.0f, 0.0f, 1.0f));

		//scale with distance of mouse from center
		model = glm::scale(model, scaleFactor);

		return model;
	}
	/*
	**returns the distance between 2 coords
	*untested in screen coords
	*/
	float get_Distance(const glm::vec2& P1, const glm::vec2& P2)
	{
		return glm::sqrt(
			(((SCR_WIDTH / 2.0f) * glm::abs(P2.x - P1.x)) * ((SCR_WIDTH / 2.0f) * glm::abs(P2.x - P1.x))) +
			(((SCR_HEIGHT / 2.0f) * glm::abs(P2.y - P1.y)) * ((SCR_HEIGHT / 2.0f) * glm::abs(P2.y - P1.y))));
	}


	//******************************* UNTESTED

	//iscollided, closestPoint
	std::pair<bool, glm::vec3> checkCollision(glm::vec3 ballPos, float ballR, glm::vec2 boxColl, glm::vec3 boxPos)
	{
		//center of the ball - we dont add radius because rect class already does that
		glm::vec2 ballCenter(ballPos.x, ballPos.y);
		//calculate aabb info (center , half extends)
		glm::vec2 aabb_half_extends(boxColl.x / 2.0f, boxColl.y / 2.0f);
		//we dont add the half extends to find the center bcuz our rect class finds the center while creating vertices so we dont need to do ut again
		glm::vec2 aabb_center(boxPos.x, boxPos.y);
		//get difference between both centers and clamp it to find closest point to ball on border
		glm::vec2 difference = ballCenter - aabb_center;
		glm::vec2 clamped = glm::clamp(difference, -aabb_half_extends, aabb_half_extends);
		//add clamped value to aabb center to get closest point
		glm::vec2 closest = aabb_center + clamped;
		//check if the distance between closest point and ball center is smaller than radius
		difference = closest - ballCenter;
		if (glm::length(difference) < ballR)
			return std::make_pair(true, glm::vec3(closest, 0.0f));
		else
			return std::make_pair(false, glm::vec3(closest, 0.0f));
	}
	glm::vec3 findMtv(glm::vec3 ballPos, float ballR, glm::vec3 closestPoint)
	{
		// 1.0 - vec2(0.5 , 0.0) = (0.5 , 1.0)

		//center of the ball
		glm::vec2 ballCenter(ballPos.x, ballPos.y);
		glm::vec2 closestPToBallCent = ballCenter - glm::vec2(closestPoint.x, closestPoint.y);
		//first find the scalar value of mtv
		float penetrationLenght = ballR - glm::length(closestPToBallCent);
		//then multiply that with direction
		glm::vec2 penetration = glm::normalize(closestPToBallCent) * penetrationLenght;
		return glm::vec3(penetration, 0.0f);
	}
	glm::vec2 genScaleForColl(float targetXLenght, float targetYLenght, float containerXLenght, float containerYLenght)
	{
		glm::vec2 temp(targetXLenght / containerXLenght, targetYLenght / containerXLenght);

		return temp;
	}
	glm::vec3 calcMeetPoint(glm::vec3 ballPos, glm::vec3 ballVel, float xBorder, float yMin, float yMax)
	{
		float distanceRate = xBorder / glm::abs(ballVel.x);
		float newY = glm::clamp(ballVel.y * distanceRate, yMin, yMax) + ballPos.y;
		return glm::vec3(xBorder, newY, ballPos.z);
	}
	std::vector<glm::vec3> generateCollider(glm::vec3 Pos, float xlength = 0.2f, float ylength = 0.2f)
	{
		std::vector<glm::vec3> collider;

		collider.push_back(glm::vec3(Pos.x - (xlength / 2.0f), Pos.y - (ylength / 2.0f), Pos.y));//leftdonw
		collider.push_back(glm::vec3(Pos.x - (xlength / 2.0f), Pos.y + (ylength / 2.0f), Pos.y));//leftup
		collider.push_back(glm::vec3(Pos.x + (xlength / 2.0f), Pos.y + (ylength / 2.0f), Pos.y));//rightup
		collider.push_back(glm::vec3(Pos.x + (xlength / 2.0f), Pos.y - (ylength / 2.0f), Pos.y));//rightdown

		return collider;
	}
	//collider order is leftdown,leftup,rightup,rightdown
	//TODO make it return the point of collision
	bool pointToRectCollision(std::vector<glm::vec3> collider, glm::vec3 point)
	{
		glm::vec3 leftDown = collider[0];
		glm::vec3 leftUp = collider[1];
		glm::vec3 rightUp = collider[2];
		glm::vec3 rightDown = collider[3];
		if (leftDown.x <= point.x && rightDown.x >= point.x)
			if (leftDown.y <= point.y && leftUp.y >= point.y)
				return true;
			else
				return false;
		else
			return false;

		return false;
	}
}


//***************************** ENUMS

//how buffer is organised
enum class Buffer_Type {
	INTERLEAVED,
	CONSECUTIVE,
	SEPERATED
};
//how each seperate vertex value is organised
enum class Vertex_Type {
	POSITION,
	POSITION_TEXTUREPOS,
	POSITION_NORMAL_TEXTUREPOS
};
enum class Draw_Method {
	TRIANGLES,
	TRIANGLE_STRIP,
	LINES
};
//targeted type in one vertex value
enum class Vertex_Target {
	POSITION,
	TEXTURECOORD,
	NORMAL
};
#endif