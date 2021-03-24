#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

//we cant include "Globals.h" here because it contains a initialisation of the Camera class

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};
enum class Mouse_Button {
	RIGHT_BUTTON,
	LEFT_BUTTON,
	MIDDLE_BUTTON,
};

class Camera
{
public:
	float yaw = -90.0f;	// if it was 0.0f camera would look slightly to the right
	float pitch = 0.0f;
	float fov = 45.0f;
	float sensitivity = 0.1f;
	float movspeed = 25.5f;

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraRight;
	glm::vec3 cameraUp;
	glm::vec3 worldUp;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
	{
		cameraPos = position;
		worldUp = up;

		UpdateVectors();
	}

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocity = movspeed * deltaTime;
		if (direction == Camera_Movement::FORWARD)
			cameraPos += cameraFront * velocity;
		if (direction == Camera_Movement::BACKWARD)
			cameraPos -= cameraFront * velocity;
		if (direction == Camera_Movement::LEFT)
			cameraPos -= cameraRight * velocity;
		if (direction == Camera_Movement::RIGHT)
			cameraPos += cameraRight * velocity;
		if (direction == Camera_Movement::UP)
			cameraPos += cameraUp * velocity;
		if (direction == Camera_Movement::DOWN)
			cameraPos -= cameraUp * velocity;
	}
	void MoveCamera(Camera_Movement direction, float movVal)
	{
		if (direction == Camera_Movement::FORWARD)
			cameraPos += cameraFront * movVal;
		if (direction == Camera_Movement::BACKWARD)
			cameraPos -= cameraFront * movVal;
		if (direction == Camera_Movement::LEFT)
			cameraPos -= cameraRight * movVal;
		if (direction == Camera_Movement::RIGHT)
			cameraPos += cameraRight * movVal;
		if (direction == Camera_Movement::UP)
			cameraPos += cameraUp * movVal;
		if (direction == Camera_Movement::DOWN)
			cameraPos -= cameraUp * movVal;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		UpdateVectors();
	}

	void ProcessMouseButton(Mouse_Button button)
	{
		if (button == Mouse_Button::RIGHT_BUTTON)
		{
			if (fov == 45.0f)
				fov = 30.0f;
			else
				fov = 45.0f;
		}
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	}


	void UpdateVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
	}
};

#endif