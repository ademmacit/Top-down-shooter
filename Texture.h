#ifndef TEXTURE_H
#define TEXTURE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <vector>
#include <iostream>
#include <string.h>
#include <unordered_map>

#include "ShaderC.h"
#include "stb_image.h"
#include "Globals.h"

const std::vector<float> defaultTexCoords =
{
	0.0f,0.0f,
	0.0f,1.0f,
	1.0f,1.0f,
	1.0f,1.0f,
	1.0f,0.0f,
	0.0f,0.0f,
};

class Texture
{
public:
	Texture(std::string TexturePath, std::string sampler2Dname = "Texture0");

	//binds textrure and textureNo so we can use the textureno in setTexture function to set a sampler2D
	void bind(int TextureNo)
	{
		textureNo = TextureNo;
		glActiveTexture(GL_TEXTURE0 + TextureNo);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void activate(Shader* ActiveShader)
	{
		glActiveTexture(GL_TEXTURE0 + textureNo);
		glBindTexture(GL_TEXTURE_2D, id);
		ActiveShader->setInt(sampler2DName, textureNo);
	}

	//setting a sampler name that corresponds to shader.
	void setSamplerName(std::string sampler2Dname )
	{
		sampler2DName = sampler2Dname;
	}

	//default is Texture0
	std::string sampler2DName;

	int textureNo;
	unsigned int id;
private:


	int width, height, nrChannels;
};
Texture::Texture(std::string TexturePath, std::string sampler2Dname)
{
	sampler2DName = sampler2Dname;

	std::vector<std::string> extensionString;
	global_functions::splitString(TexturePath, extensionString, '.');

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.


	//for png
	if (extensionString[extensionString.size() - 1] == "png")
	{
		unsigned char* data = stbi_load((TexturePath).c_str(), &width, &height, &nrChannels, STBI_rgb_alpha);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			throw "Failed to load texture : " + TexturePath;

		stbi_image_free(data);
	}
	else //for other formats
	{
		unsigned char* data = stbi_load((TexturePath).c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			throw "Failed to load texture : "+TexturePath ;

		stbi_image_free(data);
	}
}

class SpriteSheet
{
public:
	SpriteSheet(std::string Path, int RowCount, int ColCount, std::string sampler2Dname = "Texture0");

	void bind(int TextureNo)
	{
		textureNo = TextureNo;
		glActiveTexture(GL_TEXTURE0 + TextureNo);
		glBindTexture(GL_TEXTURE_2D, id);
	}

	void activate(Shader* ActiveShader)
	{
		glActiveTexture(GL_TEXTURE0 + textureNo);
		glBindTexture(GL_TEXTURE_2D, id);
		ActiveShader->setInt(sampler2DName, textureNo);
	}

	//setting a sampler name that corresponds to shader.
	void setSamplerName(std::string sampler2Dname)
	{
		sampler2DName = sampler2Dname;
	}

	//for
	//2*3    *1
	//*		  *
	//1*    3*2
	std::vector<float> buildTargetSpriteTexCoords(int targetId = 0)
	{
		if (((targetId / colCount))>= rowCount)	//out of bounds
			return firstSpriteTexCoords;
		
		if (targetId >= colCount)
		{
			currentSpriteCoord[1] = targetId / colCount;
			currentSpriteCoord[0] = targetId % colCount;
		}
		else
		{
			currentSpriteCoord[1] = 0;
			currentSpriteCoord[0] = targetId;
		}
		std::vector<float> temp = firstSpriteTexCoords;
		for (size_t i = 0; i < firstSpriteTexCoords.size(); i+=2)
		{
			temp[i] = (firstSpriteTexCoords[i] + targetId * widthTex);
			temp[i +1] = (firstSpriteTexCoords[i + 1] + currentSpriteCoord[1] * heightTex);
		}
		return temp;
	}
	std::vector<float> buildNextSpriteTexCoords(int increment = 1)
	{
		if ((currentSpriteCoord[1] + ((currentSpriteCoord[0] + increment) / colCount)) >= rowCount)	//out of bounds
			return firstSpriteTexCoords;
		currentSpriteCoord[0] += increment;
		if (currentSpriteCoord[0] >= colCount)
		{
			currentSpriteCoord[1] += currentSpriteCoord[0] / colCount;
			currentSpriteCoord[0] = currentSpriteCoord[0] % colCount;
		}
		std::vector<float> temp = firstSpriteTexCoords;
		for (size_t i = 0; i < firstSpriteTexCoords.size(); i += 2)
		{
			temp[i] = (firstSpriteTexCoords[i] + currentSpriteCoord[0] * widthTex);
			temp[i + 1] = (firstSpriteTexCoords[i + 1] + currentSpriteCoord[1] * heightTex);
		}
		return temp;
	}
	std::vector<float> getTargetSpriteTexCoords(int targetId = 0)
	{
		return allTexCoords.at(targetId);
	}

	unsigned int id;
	int textureNo;
	//default is Texture0
	std::string sampler2DName;
private:



	// for the whole sheet
	unsigned int rowCount, colCount;
	// for the each sprite
	unsigned int Width, Heigh;
	// for texture coords
	float widthTex, heightTex;
	int xGapTex, yGapTex;
	// for animation
	int currentSpriteCoord[2] = {0,0};

	std::vector<float> firstSpriteTexCoords;
	std::unordered_map<int, std::vector<float>> allTexCoords;

	void buildTexCoords()
	{
		//texture coord
		firstSpriteTexCoords.push_back(0.0f);	firstSpriteTexCoords.push_back(0.0f);
		firstSpriteTexCoords.push_back(0.0f);	firstSpriteTexCoords.push_back(heightTex);
		firstSpriteTexCoords.push_back(widthTex);	firstSpriteTexCoords.push_back(heightTex);
		firstSpriteTexCoords.push_back(widthTex);	firstSpriteTexCoords.push_back(heightTex);
		firstSpriteTexCoords.push_back(widthTex);	firstSpriteTexCoords.push_back(0.0f);
		firstSpriteTexCoords.push_back(0.0f);	firstSpriteTexCoords.push_back(0.0f);

		allTexCoords.insert(std::make_pair(0, firstSpriteTexCoords));

		for (size_t i = 1; i < rowCount*colCount; i++)
		{
			allTexCoords.insert(std::make_pair(i, buildNextSpriteTexCoords()));
		}
	}
};
SpriteSheet::SpriteSheet(std::string Path,int RowCount,int ColCount, std::string sampler2Dname)
{
	int sheetWidth, sheetHeight, nrChannels;
	sampler2DName = sampler2Dname;

	std::vector<std::string> extensionString;
	global_functions::splitString(Path, extensionString, '.');


	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.


	//for png
	if (extensionString[extensionString.size() - 1] == "png")
	{
		unsigned char* data = stbi_load((Path).c_str(), &sheetWidth, &sheetHeight, &nrChannels, STBI_rgb_alpha);

		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, sheetWidth, sheetHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			throw "Failed to load sheet : " + Path;
		stbi_image_free(data);
	}
	else //for other formats
	{
		unsigned char* data = stbi_load((Path).c_str(), &sheetWidth, &sheetHeight, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, sheetWidth, sheetHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
			throw "Failed to load sheet : " + Path;
		stbi_image_free(data);
	}

	Width = sheetWidth / ColCount;
	Heigh = sheetHeight/ RowCount;
	widthTex = (float)Width / (float)sheetWidth;
	heightTex= (float)Heigh / (float)sheetHeight;
	currentSpriteCoord[0] = 0;
	currentSpriteCoord[1] = 0;
	colCount = ColCount;
	rowCount = RowCount;
	buildTexCoords();
}


class TextureHandler
{
public:
	TextureHandler(Logger *loggerPointer);
	void addTexture(std::string name, Texture text)
	{
		text.bind(nextTextureNo++);
		textures.insert(std::make_pair(name, text));
	}
	void addTexture(std::string name, std::string path)
	{
		try
		{
			Texture temp(path);
			temp.bind(nextTextureNo++);
			textures.insert(std::make_pair(name, temp));
		}
		catch (const std::string e)
		{
			logger->add(e, "Texture.h", glm::vec3(1.0f, 0.0f, 0.0f));
		}

	}

	void addSpriteSheet(std::string name, SpriteSheet SS)
	{
		SS.bind(nextTextureNo++);
		spriteSheets.insert(std::make_pair(name,SS));
	}
	void addSpriteSheet(std::string name, std::string path, int RowCount, int ColCount)
	{
		try
		{
			SpriteSheet temp(path, RowCount, ColCount);
			temp.bind(nextTextureNo++);
			spriteSheets.insert(std::make_pair(name, temp));
		}
		catch (const std::string e)
		{
			logger->add(e, "Texture.h", glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}

	
	void activateSpriteSheet(std::string name, Shader* ActiveShader)
	{

		SpriteSheet temp = spriteSheets.at(name);

		temp.activate(ActiveShader);

	}
	void activateTexture(std::string name, Shader* ActiveShader)
	{
		if (textures.find(name) != textures.end())
		{
			//hit condition
			Texture temp = textures.at(name);
			temp.activate(ActiveShader);
		}
		else
		{
			//miss condition
			Texture temp = textures.begin()->second;
			temp.activate(ActiveShader);
		}
		

	}


	//FIX
	void release(std::string name, std::string sampler2DName, Shader* ActiveShader)
	{
		try {
			Texture temp = textures.at(name);
			glActiveTexture(GL_TEXTURE0 + temp.textureNo);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(0);
		}
		catch (...)
		{
			try {
				SpriteSheet temp = spriteSheets.at(name);
				glActiveTexture(GL_TEXTURE0 + temp.textureNo);
				glBindTexture(GL_TEXTURE_2D, 0);
				glActiveTexture(0);
			}
			catch (...) {
				logger->add("cant find <"+name+"> to release", "Texture.h", glm::vec3(1.0f, 0.0f, 0.0f));
				return;
			}
		}
	}

	Texture getTexture(std::string name)
	{
		try
		{
			return textures.at(name);
		}
		catch (...)
		{
			logger->add("cant find <" + name + "> texture", "Texture.h", glm::vec3(1.0f, 0.0f, 0.0f));
			throw "cant find <" + name + "> texture";
		}
	}
	SpriteSheet getSpriteSheet(std::string name)
	{
		try
		{
			return spriteSheets.at(name);
		}
		catch (...)
		{
			logger->add("cant find <" + name + "> spriteSheet", "Texture.h", glm::vec3(1.0f, 0.0f, 0.0f));
			throw "cant find <" + name + "> spriteSheet";
		}
	}

private:
	std::unordered_map<std::string, Texture> textures;
	std::unordered_map<std::string, SpriteSheet> spriteSheets;

	Logger* logger;

	unsigned int nextTextureNo;
};
TextureHandler::TextureHandler(Logger *loggerPointer)
{
	logger = loggerPointer;
	nextTextureNo = 0;
}


#endif