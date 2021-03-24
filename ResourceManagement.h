#ifndef RESOURCEMANAGAMENT
#define RESOURCEMANAGAMENT

#include "ShaderC.h"
#include "Texture.h"
#include "Shapes2D.h"
#include "Globals.h"


//todo
/* 
*try to change the arguments in buffer functions from vectors to arrays 
	for this maybe write a stack allocated array structure yourself?
*/
struct bufferState
{
	unsigned int VBO, VAO, bufferStride;
	Buffer_Type bufferType;
	Vertex_Type vertexType;

	bufferState(Buffer_Type bType, Vertex_Type vType)
	{
		VBO = VAO = 0;
		bufferType = bType;
		vertexType = vType;


		if (bType == Buffer_Type::CONSECUTIVE)
			bufferStride = 3;
		else
		{
			switch (vType)
			{
			case Vertex_Type::POSITION:
				bufferStride = 3;
				break;
			case Vertex_Type::POSITION_TEXTUREPOS:
				bufferStride = 5;
				break;
			case Vertex_Type::POSITION_NORMAL_TEXTUREPOS:
				bufferStride = 8;
				break;
			default:
				bufferStride = 5;
				break;
			}
		}

	}
};

//buffer segments made up a buffer. Each segment have a name so it can be drawn
//seperately by calling. 
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

	bufferSegment(std::string Name, unsigned int StartIndex, unsigned int EndIndex, Draw_Method DrawMethod, std::vector<float> Vertices)
	{
		startIndex = StartIndex;
		endIndex = EndIndex;
		name = Name;
		vertices = Vertices;
		drawMethod = DrawMethod;
	}
};

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	//**************** TEXTURE ****************
	void add_texture(std::string name, std::string path)
	{
		try
		{
			Texture temp(path);
			temp.bind(nextTextureNo++);
			textures.insert(std::make_pair(name, temp));
		}
		catch (const std::string e)
		{
			logger.add(e, "ResourceManager", glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}
	void add_spriteSheet(std::string name, std::string path, int RowCount, int ColCount)
	{
		try
		{
			SpriteSheet temp(path, RowCount, ColCount);
			temp.bind(nextTextureNo++);
			spriteSheets.insert(std::make_pair(name, temp));
		}
		catch (const std::string e)
		{
			logger.add(e, "ResourceManager", glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}
	Texture* get_texture(std::string name)
	{
		if (name == "default")
			return &textures.begin()->second;

		if (textures.find(name) != textures.end())
		{
			//hit condition
			return &textures.at(name);
		}
		else
		{
			//miss condition
			return &textures.begin()->second;
		}

	}
	SpriteSheet* get_spriteSheet(std::string name)
	{
		if (name == "default")
			return &spriteSheets.begin()->second;

		if (spriteSheets.find(name) != spriteSheets.end())
		{
			//hit condition
			return &spriteSheets.at(name);
		}
		else
		{
			//miss condition
			return &spriteSheets.begin()->second;
		}
	}

	//**************** SHADER ****************
	void add_shaderProgram(std::string name, std::string vertexShaderPath, std::string fragmentShaderPath)
	{
		Shader shaderProg(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
		shaders.insert(std::make_pair(name, shaderProg));
	}
	Shader* get_shaderProgram(std::string name)
	{
		if (name == "default")
			return &shaders.begin()->second;

		if (shaders.find(name) != shaders.end())
		{
			//hit condition
			return &shaders.at(name);
		}
		else
		{
			//miss condition
			return &shaders.begin()->second;
		}
	}

	//**************** BUFFER STATE ****************
	void add_bufferState(std::string name,Buffer_Type bType, Vertex_Type vType)
	{
		bufferState tempState(bType, vType);
		bufferStates.insert(std::make_pair(name, tempState));
	}
	bufferState* get_bufferState(std::string name)
	{
		if (name == "default")
			return &bufferStates.begin()->second;

		if (bufferStates.find(name) != bufferStates.end())
		{
			//hit condition
			return &bufferStates.at(name);
		}
		else
		{
			//miss condition
			return &bufferStates.begin()->second;
		}
	}

	//**************** BUFFERS ****************
	void add_bufferSegment(std::string name, Draw_Method drawMethod,
		std::vector<float> vertices, std::vector<float> texCoords = {}, std::vector<float> normals = {})
	{
		unsigned int startIndex = 0;

		if (bufferSegments.size() == 0)
			startIndex = 0;
		else
			startIndex = bufferSegments[bufferSegments.size() - 1].endIndex;

		unsigned int endIndex = startIndex + vertices.size();
		bufferSegment tempSegment(name, startIndex, endIndex, drawMethod, vertices);
		
		tempSegment.texCoords = texCoords;
		tempSegment.normals = normals;

		bufferSegments.push_back(tempSegment);
	}
	std::vector<bufferSegment>* get_bufferSegments()
	{
		return &bufferSegments;
	}
	std::vector<float> get_fullBuffer(Buffer_Type BufferType,  Vertex_Type VertexType)
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

		return fullBuffer;

	}
	void remove_bufferSegment(std::string name)
	{
		//****************** UNTESTED

		unsigned int changeSize = 0;
		int removeIndex = -1;
		//finds the desired segment to delete and adjusts start,end indexes of the other segments
		for (size_t i = 0; i < bufferSegments.size(); i++)
		{
			if (bufferSegments[i].name == name)
			{
				changeSize = bufferSegments[i].vertices.size();
				removeIndex = i;
			}
			if (changeSize != 0)
			{
				bufferSegments[i].startIndex -= changeSize;
				bufferSegments[i].endIndex -= changeSize;
			}
			
		}

		//deletes the desired segment
		if (removeIndex == -1)
		{ 
			logger.add("could find <" + name + "> in the buffer to delete", "Window.h", glm::vec3(1.0f, 0.0f, 0.0f));
		}
		else
			bufferSegments.erase(bufferSegments.begin() + removeIndex);
	}
	//TODO FIX THIS
	void change_bufferSegment(std::string bufferStateName,std::string objectName, Vertex_Target TargetAttribute, std::vector<float> data)
	{
		bufferState targerBufferState =  bufferStates.at(bufferStateName);
		int offSet = 0;
		//update content of the VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, targerBufferState.VBO);
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
				offSet += bufferSegments[i].vertices.size() * sizeof(float);
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

private:
	unsigned int nextTextureNo;
	std::unordered_map<std::string, Texture> textures;
	std::unordered_map<std::string, SpriteSheet> spriteSheets;
	std::unordered_map<std::string, Shader> shaders;
	std::vector<bufferSegment> bufferSegments;
	std::unordered_map<std::string, bufferState> bufferStates;

};
ResourceManager::ResourceManager()
{
	nextTextureNo = 0;
}
ResourceManager::~ResourceManager()
{
}



#endif