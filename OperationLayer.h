#ifndef OPERATIONLAYER
#define OPERATIONLAYER

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <unordered_map>

#include "ResourceManagement.h"
#include "Globals.h"



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


class OpLayer
{
public:
	OpLayer(const char* windowName, unsigned int screenWidth, unsigned int screenHeight);

	void bindBufferState(std::string bufferStateName)
	{
		bufferSegments = *resourceManager.get_bufferSegments();
		activeBufferState = resourceManager.get_bufferState(bufferStateName);

		std::vector<float> verticesBuf;
		std::vector<float> texCoordsBuf;
		std::vector<float> normalsBuf;
		std::vector<float> fullBuffer;

		// with consecutive buffer we add attributes seperately then add it to the buffer. vvtt + vvtt = vvvvtttt
		if (activeBufferState->bufferType == Buffer_Type::CONSECUTIVE)
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
				verticesBuf.insert(verticesBuf.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
			if (activeBufferState->vertexType == Vertex_Type::POSITION_TEXTUREPOS)
			{
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].texCoords.begin(), bufferSegments[i].texCoords.end());
			}
			else if (activeBufferState->vertexType == Vertex_Type::POSITION_NORMAL_TEXTUREPOS)
			{
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].normals.begin(), bufferSegments[i].normals.end());
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].texCoords.begin(), bufferSegments[i].texCoords.end());
			}

			fullBuffer.insert(fullBuffer.end(), verticesBuf.begin(), verticesBuf.end());
			fullBuffer.insert(fullBuffer.end(), texCoordsBuf.begin(), texCoordsBuf.end());
		}
		else if (activeBufferState->bufferType == Buffer_Type::INTERLEAVED)
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
				fullBuffer.insert(fullBuffer.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
		}


		glGenVertexArrays(1, &activeBufferState->VAO);
		glGenBuffers(1, &activeBufferState->VBO);
		glBindVertexArray(activeBufferState->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, activeBufferState->VBO);
		glBufferData(GL_ARRAY_BUFFER, fullBuffer.size() * sizeof(float), &fullBuffer[0], GL_DYNAMIC_DRAW);
		if (activeBufferState->bufferType == Buffer_Type::CONSECUTIVE)
		{
			int texCoordOffset = 0;
			switch (activeBufferState->vertexType)
			{
			case Vertex_Type::POSITION:
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				break;
			case Vertex_Type::POSITION_TEXTUREPOS:
				texCoordOffset = verticesBuf.size();
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(texCoordOffset * sizeof(float)));
				glEnableVertexAttribArray(1);
				break;
			case Vertex_Type::POSITION_NORMAL_TEXTUREPOS:
				texCoordOffset = verticesBuf.size() + normalsBuf.size();
				//*********************************************TODO
				break;
			}
		}
		else if (activeBufferState->bufferType == Buffer_Type::INTERLEAVED)
		{
			switch (activeBufferState->vertexType)
			{
			case Vertex_Type::POSITION:
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				break;
			case Vertex_Type::POSITION_TEXTUREPOS:
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				break;
			case Vertex_Type::POSITION_NORMAL_TEXTUREPOS:
				//pos attribute
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				//normal attribute
				glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				// texture coord attribute
				glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
				glEnableVertexAttribArray(2);
				break;
			}
		}

		glEnable(GL_DEPTH_TEST);

	}

	void activateBufferState(std::string bufferStateName)
	{
		//detach perevius buffer state values
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		//bind new ones
		activeBufferState = resourceManager.get_bufferState(bufferStateName);
		glBindVertexArray(activeBufferState->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, activeBufferState->VBO);
	}
	void activateTexture(std::string textureName)
	{
		resourceManager.get_texture(textureName)->activate(activeShader);
	}
	void activateSpriteSheet(std::string spriteSheetName)
	{
		resourceManager.get_spriteSheet(spriteSheetName)->activate(activeShader);
	}
	void activateShader(std::string shaderName)
	{
		Shader* temp =  resourceManager.get_shaderProgram(shaderName);
		temp->use();
		activeShader = temp;
	}

	void draw(std::string segmentName)
	{

		if (segmentName == "all")
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
			{
				switch (bufferSegments[i].drawMethod)
				{
				case Draw_Method::TRIANGLES:
					glDrawArrays(GL_TRIANGLES, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
					break;
				case Draw_Method::LINES:
					glDrawArrays(GL_LINES, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
					break;
				case Draw_Method::TRIANGLE_STRIP:
					glDrawArrays(GL_TRIANGLE_STRIP, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
					break;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
			{
				if (bufferSegments[i].name == segmentName)
				{
					switch (bufferSegments[i].drawMethod)
					{
					case Draw_Method::TRIANGLES:
						glDrawArrays(GL_TRIANGLES, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
						break;
					case Draw_Method::LINES:
						glDrawArrays(GL_LINES, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
						break;
					case Draw_Method::TRIANGLE_STRIP:
						glDrawArrays(GL_TRIANGLE_STRIP, bufferSegments[i].startIndex / activeBufferState->bufferStride, bufferSegments[i].vertices.size() / activeBufferState->bufferStride);
						break;
					}
					break;
				}
			}
		}
	}

	// TODO this is a temp solution write batch rendering in textrender.h
	void renderLogs()
	{
		std::vector<LogEntry> &logs = logger.get_logs();

		//max 25 rows for 0.5 scale , -25 ofset for loop , 720 heigh
		// 2.0f height for each letter * 0.5 scale = 1.0f
		// 700 / 25 = 28 , so around 25

		float yOffSet = (float)SCR_HEIGHT - 25.0f;
		size_t startIndex = 0;

		// if its overflowing from screen write last n ones. n is calculated with scrHeight according to default text values these are 0.5 scale and 25.0f padding
		if (logs.size() > (unsigned)(SCR_HEIGHT / 27))
			startIndex = logs.size() - (SCR_HEIGHT / 27);
		for (size_t i = startIndex; i < logs.size(); i++)
		{
			textHandler.renderText(activeShader, logs[i].fullText, 0, yOffSet, 0.5f, logs[i].color);
			yOffSet -= 25;
		}

		//since textrender is done with its own vao and vbo we rebind our active bufferstate after it
		glBindVertexArray(activeBufferState->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, activeBufferState->VBO);
	}
	void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
	{
		textHandler.renderText(activeShader, text, x, y, scale, color);

		//since textrender is done with its own vao and vbo we rebind our active bufferstate after it
		glBindVertexArray(activeBufferState->VAO);
		glBindBuffer(GL_ARRAY_BUFFER, activeBufferState->VBO);
	}
	GLFWwindow* get_window()
	{
		return window;
	}
	void initDefaultValues()
	{

	}
	
	ResourceManager resourceManager;
	TextRenderHandler textHandler;
	Shader* activeShader = nullptr;
	bufferState* activeBufferState = nullptr;
	//this gets coppied from resource manager in bindbufferState
	std::vector<bufferSegment> bufferSegments;

private: 
	GLFWwindow* window;
};

OpLayer::OpLayer(const char* windowName, unsigned int screenWidth, unsigned int screenHeight) 
{

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(screenWidth, screenHeight, windowName, NULL, NULL);

	logger.add("window started", "Window.h");
	if (window == NULL)
	{
		logger.add("Failed to create GLFW window", "Window.h", glm::vec3(1.0f, 0.0f, 0.0f));
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger.add("Failed to initialize GLAD", "Window.h", glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	textHandler.init();

	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
}
#endif	