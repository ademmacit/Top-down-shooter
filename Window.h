#ifndef WINDOW_H
#define WINDOW_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <vector>
#include <string.h>
#include <fstream>
#include <iostream>
#include <unordered_map>

#include "ShaderC.h"
#include "Logger.h"
#include "Globals.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


struct bufferSegment
{
	std::string name;
	unsigned int startIndex;
	unsigned int endIndex;
	std::vector<float> vertices;
	Draw_Method drawMethod;

	//for consecutive buffers
	std::vector<float> texCoords;
	std::vector<float> normals;

	//for indiced buffers
	std::vector<unsigned int> indices;
	unsigned int indiceStartIndex;
	unsigned int indiceEndIndex;


	bufferSegment(std::string Name, unsigned int StartIndex, unsigned int EndIndex, Draw_Method DrawMethod, std::vector<float> Vertices)
	{
		startIndex = StartIndex;
		endIndex = EndIndex;
		name = Name;
		vertices = Vertices;
		drawMethod = DrawMethod;
	}

};

class Window
{
public:
	Window(const char* windowName,unsigned int screenWidth, unsigned int screenHeight, Logger* loggerPoint);
	//-----shader------//
	void addShader(std::string shaderName, Shader *shader)
	{
		shaders.insert(std::make_pair(shaderName, shader));
		activeShader = shader;
	}
	void useShader(std::string shaderName)
	{
		shaders.at(shaderName)->use();
		activeShader = shaders.at(shaderName);			
	}
	void useActiveShader()
	{
		activeShader->use();
	}

	//-----buffer------//
	void addToBuffer(std::string objectName, Draw_Method drawMethod, std::vector<float> interleavedVertices)
	{
		unsigned int startIndex = 0;
		
		if (bufferSegments.size()==0)
			startIndex = 0;
		else
			startIndex = bufferSegments[bufferSegments.size() - 1].endIndex;
		
		unsigned int endIndex = startIndex + interleavedVertices.size();
		bufferSegment tempSegment(objectName,startIndex,endIndex, drawMethod, interleavedVertices);
		
		bufferSegments.push_back(tempSegment);
	}
	void addToBuffer(std::string objectName, Draw_Method drawMethod, std::vector<float> vertices, std::vector<float> texCoords)
	{
		unsigned int startIndex = 0;

		if (bufferSegments.size() == 0)
			startIndex = 0;
		else
			startIndex = bufferSegments[bufferSegments.size() - 1].endIndex;

		unsigned int endIndex = startIndex + vertices.size();
		bufferSegment tempSegment(objectName, startIndex, endIndex, drawMethod, vertices);
		tempSegment.texCoords = texCoords;

		bufferSegments.push_back(tempSegment);
	}
	void addToBuffer(std::string objectName, Draw_Method drawMethod, std::vector<float> vertices, std::vector<float> texCoords, std::vector<float> normals)
	{
		unsigned int startIndex = 0;

		if (bufferSegments.size() == 0)
			startIndex = 0;
		else
			startIndex = bufferSegments[bufferSegments.size() - 1].endIndex;

		unsigned int endIndex = startIndex + vertices.size();
		bufferSegment tempSegment(objectName, startIndex, endIndex, drawMethod, vertices);
		tempSegment.texCoords = texCoords;
		tempSegment.normals = normals;

		bufferSegments.push_back(tempSegment);
	}
	
	//creates a buffer segment from vertices and indices. default draw type is position_normal_texture
	void addToBufferIndice(std::string objectName, Draw_Method drawMethod, std::vector<float> vertices,std::vector<unsigned int> indices)
	{
		unsigned int startIndex = 0;
		unsigned int indiceStartIndex = 0;


		if (bufferSegments.size() != 0)
		{
			startIndex = bufferSegments[bufferSegments.size() - 1].endIndex;
			indiceStartIndex = bufferSegments[bufferSegments.size() - 1].indiceEndIndex;
		}


		unsigned int indiceEndIndex = indiceStartIndex + indices.size();
		unsigned int endIndex = startIndex + vertices.size();
		bufferSegment tempSegment(objectName, startIndex, endIndex, drawMethod, vertices);

		tempSegment.indices = indices;
		tempSegment.indiceStartIndex = indiceStartIndex;
		tempSegment.indiceEndIndex = indiceEndIndex;


		bufferSegments.push_back(tempSegment);
	}
	void removeFromBuffer(std::string objectName)
	{
		unsigned int changeSize = 0;
		int removeIndex = -1;
		//finds the desired segment to delete and adjusts start,end indexes of the other segments
		for (size_t i = 0; i < bufferSegments.size(); i++)
		{
			if (bufferSegments[i].name == objectName)
			{
				changeSize = bufferSegments[i].vertices.size();
				removeIndex = i;
			}
			bufferSegments[i].startIndex -= changeSize;
			bufferSegments[i].endIndex -= changeSize;
		}

		//deletes the desired segment
		if (removeIndex == -1)
			logger->add("could find <"+objectName+"> in the buffer to delete","Window.h",glm::vec3(1.0f,0.0f,0.0f));
		else
			bufferSegments.erase(bufferSegments.begin() + removeIndex);
	}
	void clearVertexBuffer()
	{
		bufferSegments.clear();
	}
	std::vector<float> getBuffer(int BufferStride)
	{
		bufferStride = BufferStride;
		std::vector<float> buffer;
		for (size_t i = 0; i < bufferSegments.size(); i++)
			buffer.insert(buffer.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());

		return buffer;
	}
	void changeBuffer(std::string objectName,Vertex_Target TargetAttribute, std::vector<float> data)
	{
		int offSet = 0;
		//update content of the VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		switch (TargetAttribute)
		{
		case Vertex_Target::POSITION:	//UNTESTED

			for (size_t i = 0; i < bufferSegments.size(); i++)
			{
				if (objectName == bufferSegments[i].name)
					break;
				else
					offSet += bufferSegments[i].vertices.size() * sizeof(float);
			}
			break;
		case Vertex_Target::TEXTURECOORD:
			for (size_t i = 0; i < bufferSegments.size(); i++)
				offSet+= bufferSegments[i].vertices.size() * sizeof(float);
			for (size_t i = 0; i < bufferSegments.size(); i++)
			{
				if (objectName == bufferSegments[i].name)
					break;
				else
					offSet += bufferSegments[i].texCoords.size() * sizeof(float);
			}
			break;
		case Vertex_Target::NORMAL: //since normal is in middle PositionNormalTexcoord it is same with TEXTURECOORD //UNTESTED
			for (size_t i = 0; i < bufferSegments.size(); i++)
				offSet += bufferSegments[i].vertices.size() * sizeof(float);
			for (size_t i = 0; i < bufferSegments.size(); i++)
			{
				if (objectName == bufferSegments[i].name)
					break;
				else
					offSet += bufferSegments[i].texCoords.size() * sizeof(float);
			}
			break;
		}
		glBufferSubData(GL_ARRAY_BUFFER, offSet, data.size() * sizeof(float), &data[0]);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	//-----vao------//
	void vao(Buffer_Type BufferType, Vertex_Type VertexType)
	{
		std::vector<float> verticesBuf;
		std::vector<float> texCoordsBuf;
		std::vector<float> normalsBuf;
		std::vector<float> fullBuffer;

		// with consecutive buffer we add attributes seperately then add it to the buffer. vvtt + vvtt = vvvvtttt
		if (BufferType == Buffer_Type::CONSECUTIVE)
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
				verticesBuf.insert(verticesBuf.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
			if (VertexType == Vertex_Type::POSITION_TEXTUREPOS)
			{
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].texCoords.begin(), bufferSegments[i].texCoords.end());
			}
			else if (VertexType == Vertex_Type::POSITION_NORMAL_TEXTUREPOS)
			{
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].normals.begin(), bufferSegments[i].normals.end());
				for (size_t i = 0; i < bufferSegments.size(); i++)
					texCoordsBuf.insert(texCoordsBuf.end(), bufferSegments[i].texCoords.begin(), bufferSegments[i].texCoords.end());
			}

			fullBuffer.insert(fullBuffer.end(), verticesBuf.begin(), verticesBuf.end());
			fullBuffer.insert(fullBuffer.end(), texCoordsBuf.begin(), texCoordsBuf.end());
		}
		else if (BufferType == Buffer_Type::INTERLEAVED)
		{
			for (size_t i = 0; i < bufferSegments.size(); i++)
				fullBuffer.insert(fullBuffer.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
		}




		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, fullBuffer.size() * sizeof(float), &fullBuffer[0], GL_DYNAMIC_DRAW);

		if (BufferType == Buffer_Type::CONSECUTIVE)
		{
			bufferStride = 3;
			int texCoordOffset = 0;
			switch (VertexType)
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
		else if (BufferType == Buffer_Type::INTERLEAVED)
		{
			switch (VertexType)
			{
			case Vertex_Type::POSITION:
				bufferStride = 3;
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				break;
			case Vertex_Type::POSITION_TEXTUREPOS:
				bufferStride = 5;
				glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
				glEnableVertexAttribArray(0);
				// texture coord attribute
				glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
				glEnableVertexAttribArray(1);
				break;
			case Vertex_Type::POSITION_NORMAL_TEXTUREPOS:
				bufferStride = 8;
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

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
	}
	void vaoIndice(Vertex_Type Type)
	{
		//creating a buffer from segments to give vao
		std::vector<float> buffer;
		std::vector<unsigned int> indiceBuffer;

		for (size_t i = 0; i < bufferSegments.size(); i++)
		{
			if (i == 0)
			{
				buffer.insert(buffer.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
				indiceBuffer.insert(indiceBuffer.end(), bufferSegments[i].indices.begin(), bufferSegments[i].indices.end());
			}
			else
			{
				//manualy setting all of the indices in the buffersegment to start from buffer's end.
				for (size_t j = 0; j < bufferSegments[i].indices.size(); j++)
				{
					bufferSegments[i].indices[j] += buffer.size()/8;
				}
				buffer.insert(buffer.end(), bufferSegments[i].vertices.begin(), bufferSegments[i].vertices.end());
				indiceBuffer.insert(indiceBuffer.end(), bufferSegments[i].indices.begin(), bufferSegments[i].indices.end());
			}
			
		}
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), &buffer[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indiceBuffer.size()*sizeof(unsigned int), &indiceBuffer[0], GL_STATIC_DRAW);

		if (Type == Vertex_Type::POSITION)
		{
			bufferStride = 3;
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
		}
		else if (Type == Vertex_Type::POSITION_TEXTUREPOS)
		{
			bufferStride = 5;
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			// texture coord attribute
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
		}
		else if (Type == Vertex_Type::POSITION_NORMAL_TEXTUREPOS)
		{
			bufferStride = 8;
			//pos attribute
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			//normal attribute
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);
			// texture coord attribute
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
			glEnableVertexAttribArray(2);
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glEnable(GL_DEPTH_TEST);
	}
	void draw(std::string objectName)
	{
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		for (size_t i = 0; i < bufferSegments.size(); i++)
		{
			if (bufferSegments[i].name == objectName)
			{
				switch (bufferSegments[i].drawMethod)
				{
				case Draw_Method::TRIANGLES:
					glDrawArrays(GL_TRIANGLES, bufferSegments[i].startIndex / bufferStride, bufferSegments[i].vertices.size() / bufferStride);
					break;
				case Draw_Method::LINES:
					glDrawArrays(GL_LINES, bufferSegments[i].startIndex / bufferStride, bufferSegments[i].vertices.size() / bufferStride);
					break;
				case Draw_Method::TRIANGLE_STRIP:
					glDrawArrays(GL_TRIANGLE_STRIP, bufferSegments[i].startIndex / bufferStride, bufferSegments[i].vertices.size() / bufferStride);
					break;
				}
			}
		}
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}
	void drawIndices(std::string objectName)
	{
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		//drawElemets(drawtype,//, number of elements to draw.ie indeces.size,//, typle of indices,//, start of the indices to draw in size.ie (void*)5*sizeof(unsigned int))
		for (size_t i = 0; i < bufferSegments.size(); i++)
		{
			if (bufferSegments[i].name == objectName)
			{
				switch (bufferSegments[i].drawMethod)
				{
				case Draw_Method::TRIANGLES:
					glDrawElements(GL_TRIANGLES, bufferSegments[i].indices.size(), GL_UNSIGNED_INT, (void*)(bufferSegments[i].indiceStartIndex*sizeof(unsigned int)));
					break;
				case Draw_Method::LINES:
					glDrawElements(GL_LINES, bufferSegments[i].indices.size(), GL_UNSIGNED_INT, (void*)(bufferSegments[i].indiceStartIndex*sizeof(unsigned int)));
					break;
				case Draw_Method::TRIANGLE_STRIP:
					glDrawElements(GL_TRIANGLE_STRIP, bufferSegments[i].indices.size(), GL_UNSIGNED_INT, (void*)(bufferSegments[i].indiceStartIndex*sizeof(unsigned int)));
					break;
				}

			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	}

	//-----utility------//
	void setDrawType(Vertex_Type Type)
	{
		if (Type == Vertex_Type::POSITION)
			bufferStride = 3;
		else if (Type == Vertex_Type::POSITION_TEXTUREPOS)
			bufferStride = 5;
		else if (Type == Vertex_Type::POSITION_NORMAL_TEXTUREPOS)
			bufferStride = 8;
	}
	void setClearColor(std::vector<float> color)
	{
		glClearColor(color[0], color[1], color[2], color[3]);
	}
	GLFWwindow* getWindow()
	{
		return window;
	}
	void clearObjectBuffers()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);

	}


	Shader* activeShader;
private:
	unsigned int VBO, VAO, EBO;

	GLFWwindow* window;
	std::unordered_map<std::string, Shader*> shaders;
	
	Logger *logger;

	//stores float count per row in the buffer for draw function
	int bufferStride = 8;
	std::vector<bufferSegment> bufferSegments;
};
Window::Window(const char* windowName, unsigned int screenWidth, unsigned int screenHeight,Logger* loggerPoint)
{
	VBO = VAO = EBO = 0;
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	logger = loggerPoint;

	window = glfwCreateWindow(screenWidth, screenHeight, windowName, NULL, NULL);
	logger->add("window started", "Window.h", glm::vec3(0.0f, 0.0f, 0.0f));
	if (window == NULL)
	{
		logger->add("Failed to create GLFW window", "Window.h", glm::vec3(1.0f, 0.0f, 0.0f));
		glfwTerminate();
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		logger->add("Failed to initialize GLAD", "Window.h", glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);


	glClearColor(0.1f, 0.2f, 0.2f, 1.0f);
}


#endif