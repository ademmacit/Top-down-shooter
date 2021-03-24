#ifndef SHAPES_H
#define SHAPES_H

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <vector>

#include "Globals.h"


/*___________TODO
3d  
	^add building interleavedvertices type distinction to cuboid and tube like rectagle
	make show vertex normals mode
*/



glm::vec3 findPerpendicular(glm::vec3 vector)
{
	//x*1 +y*1 + z*newZ = 0

	//find the perpendicular of the rotation angle
	glm::vec3 perpendicular;
	glm::vec3 mult = glm::vec3(1.0f);
	/*
	since perpendicular of a vector with below 0 x is in the opposite direction than
	a vector with above 0 x we check the condition and reverse for smooth transition
	*/
	if (vector.x < 0)
	{
		mult *= glm::vec3(-1.0f, -1.0f, -1.0f);
	}

	if (vector.x != 0)
	{

		float newX = (vector.y + vector.z) / -vector.x;
		perpendicular = glm::normalize(glm::vec3(newX, 1.0f, 1.0f)*mult);
	}
	else if (vector.y != 0)
	{

		float newY = (vector.x + vector.z) / -vector.y;
		perpendicular = glm::normalize(glm::vec3(1.0f, newY, 1.0f)*mult);
	}
	else
	{
		float newZ = (vector.x + vector.y) / -vector.z;
		perpendicular = glm::normalize(glm::vec3(1.0f, 1.0f, newZ)*mult);

	}
	return perpendicular;
}
glm::vec3 findPerpendicular(glm::vec3 P1, glm::vec3 P2)
{
	//x*1 +y*1 + z*newZ = 0
	glm::vec3 vector = glm::vec3(P1.x - P2.x, P1.y - P2.y, P1.z - P2.z);
	vector = glm::normalize(vector);

	//find the perpendicular of the rotation angle
	glm::vec3 perpendicular;
	if (vector.x != 0)
	{
		float newX = (vector.y + vector.z) / -vector.x;
		perpendicular = glm::normalize(glm::vec3(newX, 1.0f, 1.0f));
	}
	else if (vector.y != 0)
	{
		float newY = (vector.x + vector.z) / -vector.y;
		perpendicular = glm::normalize(glm::vec3(1.0f, newY, 1.0f));
	}
	else
	{
		float newZ = (vector.x + vector.y) / -vector.z;
		perpendicular = glm::normalize(glm::vec3(1.0f, 1.0f, newZ));
	}

	return perpendicular;
}

struct LINE
{

	glm::vec3 P1;
	glm::vec3 P2;
	Vertex_Type vType;
	float length;

	std::vector<float> vertices;
	std::vector<float> texCoords;
	std::vector<float> interleavedVertices;

	std::vector<float> getVertices()
	{
		return vertices;
	}
	std::vector<float> getTexCoords()
	{
		return texCoords;
	}

	std::vector<float> getInterleavedVertices()
	{
		return interleavedVertices;
	}

	void build()
	{
		vertices.push_back(P1.x);
		vertices.push_back(P1.y);
		vertices.push_back(P1.z);

		vertices.push_back(P2.x);
		vertices.push_back(P2.y);
		vertices.push_back(P2.z);

		texCoords.push_back(0.0f);
		texCoords.push_back(0.0f);

		texCoords.push_back(1.0f);
		texCoords.push_back(1.0f);

		length = glm::abs(P1.x - P2.x);

		if (vType == Vertex_Type::POSITION)
		{
			interleavedVertices = vertices;
		}
		else if (vType == Vertex_Type::POSITION_TEXTUREPOS)
		{
			interleavedVertices.push_back(vertices[0]);
			interleavedVertices.push_back(vertices[1]);
			interleavedVertices.push_back(vertices[2]);

			interleavedVertices.push_back(texCoords[0]);
			interleavedVertices.push_back(texCoords[1]);

			interleavedVertices.push_back(vertices[3]);
			interleavedVertices.push_back(vertices[4]);
			interleavedVertices.push_back(vertices[5]);

			interleavedVertices.push_back(texCoords[2]);
			interleavedVertices.push_back(texCoords[3]);
		}
	}

	void rebuild(glm::vec3 point1, glm::vec3 point2)
	{
		P1 = point1;
		P2 = point2;
		build();
	}

	LINE(Vertex_Type v_Type, glm::vec3 point1, glm::vec3 point2)
	{
		P1 = point1;
		P2 = point2;
		vType = v_Type;

		build();
	}
};

//rectangle was taken
class Rect
{
public:
	std::vector<float> getVertices()
	{
		return vertices;
	}
	std::vector<float> getTexCoords()
	{
		return texCoords;
	}
	std::vector<float> getNormals()
	{
		return normals;
	}
	std::vector<float> getInterleavedVertices()
	{
		return interleavedVertices;
	}

	Rect(Vertex_Type vType, float XLenght = 1.0f, float YLenght = 1.0f, glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f));
	void buildPlane(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4, glm::vec3 NormalOfPlane)
	{

		//--------vertices-------//

		//position																			//normal	
		vertices.push_back(P1.x);	vertices.push_back(P1.y);	vertices.push_back(P1.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		//texture coord
		texCoords.push_back(0.0f);	texCoords.push_back(0.0f);

		vertices.push_back(P2.x);	vertices.push_back(P2.y);	vertices.push_back(P2.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(0.0f);	texCoords.push_back(1.0f);

		vertices.push_back(P3.x);	vertices.push_back(P3.y);	vertices.push_back(P3.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(1.0f);	texCoords.push_back(1.0f);

		vertices.push_back(P3.x);	vertices.push_back(P3.y);	vertices.push_back(P3.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(1.0f);	texCoords.push_back(1.0f);

		vertices.push_back(P4.x);	vertices.push_back(P4.y);	vertices.push_back(P4.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(1.0f);	texCoords.push_back(0.0f);

		vertices.push_back(P1.x);	vertices.push_back(P1.y);	vertices.push_back(P1.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(0.0f);	texCoords.push_back(0.0f);
	}
	void buildInterleavedVertices(Vertex_Type vType)
	{
		interleavedVertices.clear();
		std::size_t i, j;
		std::size_t count = vertices.size();

		switch (vType)
		{
		case Vertex_Type::POSITION:
			for (i = 0, j = 0; i < count; i += 3, j += 2)
			{
				interleavedVertices.push_back(vertices[i]);
				interleavedVertices.push_back(vertices[i + 1]);
				interleavedVertices.push_back(vertices[i + 2]);
			}
			break;
		case Vertex_Type::POSITION_TEXTUREPOS:
			for (i = 0, j = 0; i < count; i += 3, j += 2)
			{
				interleavedVertices.push_back(vertices[i]);
				interleavedVertices.push_back(vertices[i + 1]);
				interleavedVertices.push_back(vertices[i + 2]);

				interleavedVertices.push_back(texCoords[j]);
				interleavedVertices.push_back(texCoords[j + 1]);
			}
			break;
		case Vertex_Type::POSITION_NORMAL_TEXTUREPOS:
			for (i = 0, j = 0; i < count; i += 3, j += 2)
			{
				interleavedVertices.push_back(vertices[i]);
				interleavedVertices.push_back(vertices[i + 1]);
				interleavedVertices.push_back(vertices[i + 2]);

				interleavedVertices.push_back(normals[i]);
				interleavedVertices.push_back(normals[i + 1]);
				interleavedVertices.push_back(normals[i + 2]);

				interleavedVertices.push_back(texCoords[j]);
				interleavedVertices.push_back(texCoords[j + 1]);
			}
			break;
		}
	}

private:
	float xLenght;
	float yLenght;
	glm::vec3 position;

	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<float> interleavedVertices;
};
Rect::Rect(Vertex_Type vType,float XLenght, float YLenght, glm::vec3 Position)
{
	xLenght = XLenght;
	yLenght = YLenght;
	position = Position;

	std::vector<glm::vec3> points;
	//left down
	points.push_back(position + glm::vec3(-xLenght / 2.0f, -yLenght / 2.0f, 0.0f));
	//left up
	points.push_back(position + glm::vec3(-xLenght / 2.0f, yLenght / 2.0f, 0.0f));
	//right up
	points.push_back(position + glm::vec3(xLenght / 2.0f, yLenght / 2.0f, 0.0f));
	//right down
	points.push_back(position + glm::vec3(xLenght / 2.0f, -yLenght / 2.0f, 0.0f));

	buildPlane(points[0], points[1], points[2], points[3], glm::vec3(0.0f, 0.0f, 1.0f));

	buildInterleavedVertices(vType);
}

class Cuboid
{
public:
	Cuboid(glm::vec3 Position, float Length, float Width, float Height);

	std::vector<unsigned int> getIndices()
	{
		return indices;
	}
	std::vector<float> getVertices()
	{
		return vertices;
	}
	std::vector<float> getTexCoords()
	{
		return texCoords;
	}
	std::vector<float> getNormals()
	{
		return normals;
	}
	std::vector<float> getInterleavedVertices()
	{
		return interleavedVertices;
	}

	void buildInterleavedVertices()
	{
		interleavedVertices.clear();
		std::size_t i, j;
		std::size_t count = vertices.size();
		for (i = 0, j = 0; i < count; i += 3, j += 2)
		{
			interleavedVertices.push_back(vertices[i]);
			interleavedVertices.push_back(vertices[i + 1]);
			interleavedVertices.push_back(vertices[i + 2]);

			interleavedVertices.push_back(normals[i]);
			interleavedVertices.push_back(normals[i + 1]);
			interleavedVertices.push_back(normals[i + 2]);

			interleavedVertices.push_back(texCoords[j]);
			interleavedVertices.push_back(texCoords[j + 1]);
		}

	}
	void buildPlane(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, glm::vec3 P4, glm::vec3 NormalOfPlane, unsigned int indiceOffset)
	{

		//--------vertices-------//
		//position																			//normal	
		vertices.push_back(P1.x);	vertices.push_back(P1.y);	vertices.push_back(P1.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		//texture coord
		texCoords.push_back(0.0f);	texCoords.push_back(0.0f);

		vertices.push_back(P2.x);	vertices.push_back(P2.y);	vertices.push_back(P2.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(0.0f);	texCoords.push_back(1.0f);

		vertices.push_back(P3.x);	vertices.push_back(P3.y);	vertices.push_back(P3.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(1.0f);	texCoords.push_back(1.0f);

		vertices.push_back(P4.x);	vertices.push_back(P4.y);	vertices.push_back(P4.z);	normals.push_back(NormalOfPlane.x);	normals.push_back(NormalOfPlane.y);	normals.push_back(NormalOfPlane.z);
		texCoords.push_back(1.0f);	texCoords.push_back(0.0f);

		//--------indices-------//
		indices.push_back(0 + indiceOffset);	indices.push_back(1 + indiceOffset);	indices.push_back(2 + indiceOffset); //left upper triangle
		indices.push_back(2 + indiceOffset);   indices.push_back(3 + indiceOffset);	indices.push_back(0 + indiceOffset); //right lower triangle
	}

private:
	std::vector<float> vertices;
	std::vector<float> normals;
	std::vector<float> texCoords;
	std::vector<unsigned int> indices;
	std::vector<float> interleavedVertices;
	glm::vec3 position;
	float length;
	float width;
	float heigth;
};
Cuboid::Cuboid(glm::vec3 Position, float Length, float Width, float Height)
{
	//setting class variables
	position = Position;
	length = Length;
	width = Width;
	heigth = Height;

	std::vector<glm::vec3> points;

	//finding main vertices of the cuboid

	//0 up left front
	points.push_back(glm::vec3(position + glm::vec3(-length / 2.0f, heigth / 2.0f, width / 2.0f)));
	//1 up left back
	points.push_back(glm::vec3(position + glm::vec3(-length / 2.0f, heigth / 2.0f, -width / 2.0f)));
	//2 up right back
	points.push_back(glm::vec3(position + glm::vec3(length / 2.0f, heigth / 2.0f, -width / 2.0f)));
	//3 up right front
	points.push_back(glm::vec3(position + glm::vec3(length / 2.0f, heigth / 2.0f, width / 2.0f)));

	//4 down left front
	points.push_back(glm::vec3(position + glm::vec3(-length / 2.0f, -heigth / 2.0f, width / 2.0f)));
	//5 down left back
	points.push_back(glm::vec3(position + glm::vec3(-length / 2.0f, -heigth / 2.0f, -width / 2.0f)));
	//6 down right back
	points.push_back(glm::vec3(position + glm::vec3(length / 2.0f, -heigth / 2.0f, -width / 2.0f)));
	//7 down right front
	points.push_back(glm::vec3(position + glm::vec3(length / 2.0f, -heigth / 2.0f, width / 2.0f)));

	// for normals
	glm::vec3 upVector(0.0f, 1.0f, 0.0f);
	glm::vec3 downVector(0.0f, -1.0f, 0.0f);
	glm::vec3 leftVector(-1.0f, 0.0f, 0.0f);
	glm::vec3 rightVector(1.0f, 0.0f, 0.0f);
	glm::vec3 frontVector(0.0f, 0.0f, -1.0f);
	glm::vec3 backVector(0.0f, 0.0f, 1.0f);


	//up
	buildPlane(points[0], points[1], points[2], points[3], upVector, 0);
	//down
	buildPlane(points[4], points[7], points[6], points[5], downVector, vertices.size() / 3);
	//left
	buildPlane(points[1], points[0], points[4], points[5], leftVector, vertices.size() / 3);
	//right
	buildPlane(points[3], points[2], points[6], points[7], rightVector, vertices.size() / 3);
	//front
	buildPlane(points[0], points[3], points[7], points[4], frontVector, vertices.size() / 3);
	//back
	buildPlane(points[2], points[1], points[5], points[6], backVector, vertices.size() / 3);
	

	//finally combining vertices normals and texcoords in interleaved vertices
	buildInterleavedVertices();
}

/*=====ISSUES======
----doesnt display the tube if its the only object
----texturing doesnt work properly
----tearing at the start of the tube
----no radius control
*/
class Tube
{
public:
	Tube(std::vector<glm::vec3> mainLine, int cornerCount, float radius);
	std::vector<unsigned int> getIndices()
	{
		return indices;
	}
	std::vector<float> getVertices()
	{
		return vertices;
	}

	//utility for tubing
	std::pair<glm::vec3, glm::vec3> findPerp_RotAngle(glm::vec3 P1, glm::vec3 P2)
	{
		glm::vec3 rotationAngle = glm::vec3(P1.x - P2.x, P1.y - P2.y, P1.z - P2.z);
		rotationAngle = glm::normalize(rotationAngle);
		glm::vec3 perpendicular = findPerpendicular(rotationAngle);

		return std::make_pair(perpendicular, rotationAngle);
	}
	std::pair<glm::vec3, glm::vec3> findPerp_RotAngle(glm::vec3 P1, glm::vec3 P2, glm::vec3 P3)
	{

		//find the perpendicular between 1,2 and 2,3
		glm::vec3 rotationAngle1 = glm::vec3(P1.x - P2.x, P1.y - P2.y, P1.z - P2.z);
		rotationAngle1 = glm::normalize(rotationAngle1);
		glm::vec3 perpendicular1 = findPerpendicular(rotationAngle1);

		glm::vec3 rotationAngle2 = glm::vec3(P2.x - P3.x, P2.y - P3.y, P2.z - P3.z);
		rotationAngle2 = glm::normalize(rotationAngle2);
		glm::vec3 perpendicular2 = findPerpendicular(rotationAngle2);

		//get perpendicular by finding vector between two parts. rotation angle by geting its perpendicular
		glm::vec3 perpendicular = glm::normalize(perpendicular2 + perpendicular1);
		glm::vec3 rotationAngle = glm::normalize(rotationAngle1 + rotationAngle2);


		return std::make_pair(perpendicular, rotationAngle);
	}
	std::vector<float> createTubeFull(std::vector<glm::vec3> mainLine, int cornerCount, float radius)
	{
		//texture
		std::vector<std::vector<float>> texCoords{ { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };
		int texCoordCounter = 0;

		std::vector<float> TubeVertices;
		//gets the perpendicular and rotationangle needed to create first circle
		std::pair<glm::vec3, glm::vec3> lastPerp_RotAngle = findPerp_RotAngle(mainLine[0], mainLine[1]);
		std::pair<glm::vec3, glm::vec3> CurrPerp_RotAngle;

		float angleOffsetInc = 360.0f / cornerCount;
		for (unsigned int i = 1; i < mainLine.size(); i += 1)
		{

			//second aurguments to create second circle for strip
			if (i == mainLine.size() - 1)
				CurrPerp_RotAngle = findPerp_RotAngle(mainLine[i - 1], mainLine[i]);
			else
				CurrPerp_RotAngle = findPerp_RotAngle(mainLine[i - 1], mainLine[i], mainLine[i + 1]);


			float angleOffset = 0;
			for (float t = 0; t < cornerCount + 1; t++)
			{
				//first circle with rotation and translating to vertex pos --used last iterations aurguments
				glm::mat4 model = glm::mat4(1.0f);;
				model = glm::translate(model, mainLine[i - 1]);
				model = glm::rotate(model, glm::radians(angleOffset), lastPerp_RotAngle.second);
				glm::vec3 fin = glm::vec3(model * glm::vec4(lastPerp_RotAngle.first, 1.0f));

				TubeVertices.push_back(fin.x);
				TubeVertices.push_back(fin.y);
				TubeVertices.push_back(fin.z);
				//normal
				TubeVertices.push_back(lastPerp_RotAngle.first.x);
				TubeVertices.push_back(lastPerp_RotAngle.first.y);
				TubeVertices.push_back(lastPerp_RotAngle.first.z);
				//tex
				TubeVertices.push_back(texCoords[texCoordCounter][0]);
				TubeVertices.push_back(texCoords[texCoordCounter][1]);
				texCoordCounter++;

				//second circle
				model = glm::mat4(1.0f);;
				model = glm::translate(model, mainLine[i]);
				model = glm::rotate(model, glm::radians(angleOffset), CurrPerp_RotAngle.second);
				fin = glm::vec3(model * glm::vec4(CurrPerp_RotAngle.first, 1.0f));

				TubeVertices.push_back(fin.x);
				TubeVertices.push_back(fin.y);
				TubeVertices.push_back(fin.z);
				//normal
				TubeVertices.push_back(CurrPerp_RotAngle.first.x);
				TubeVertices.push_back(CurrPerp_RotAngle.first.y);
				TubeVertices.push_back(CurrPerp_RotAngle.first.z);
				//tex
				TubeVertices.push_back(texCoords[texCoordCounter][0]);
				TubeVertices.push_back(texCoords[texCoordCounter][1]);

				if (texCoordCounter == 3)
					texCoordCounter = 0;
				else
					texCoordCounter++;


				angleOffset += angleOffsetInc;
			}
			//need to add each circle two times for triangle strip
			lastPerp_RotAngle = CurrPerp_RotAngle;
		}
		return TubeVertices;
	}

private:
	std::vector<float> vertices;
	std::vector<unsigned int> indices;
	std::vector<glm::vec3> line;
	int cornerCount;
	float radius;
};
Tube::Tube(std::vector<glm::vec3> mainLine, int CornerCount, float Radius)
{
	line = mainLine;
	cornerCount = CornerCount;
	radius = Radius;

	//texture
	std::vector<std::vector<float>> texCoords{ { 0.0f, 1.0f }, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f } };
	int texCoordCounter = 0;

	std::vector<float> TubeVertices;
	std::vector<unsigned int> TubeIndices;
	unsigned int indiceCounter1 = 0;
	unsigned int indiceCounter2 = CornerCount;


	//gets the perpendicular and rotationangle needed to create first circle
	std::pair<glm::vec3, glm::vec3> CurrPerp_RotAngle;


	float angleOffsetInc = 360.0f / cornerCount;
	for (unsigned int i = 0; i < mainLine.size(); i++)
	{
		//second aurguments to create second circle for strip
		if (i == mainLine.size() - 1)
			CurrPerp_RotAngle = findPerp_RotAngle(mainLine[i - 1], mainLine[i]);
		else if (i == 0)
			CurrPerp_RotAngle = findPerp_RotAngle(mainLine[i], mainLine[i + 1]);
		else
			CurrPerp_RotAngle = findPerp_RotAngle(mainLine[i - 1], mainLine[i], mainLine[i + 1]);

		float angleOffset = 0;
		for (float t = 0; t < cornerCount; t++)
		{
			glm::mat4 model = glm::mat4(1.0f);;
			model = glm::translate(model, mainLine[i]);
			model = glm::rotate(model, glm::radians(angleOffset), CurrPerp_RotAngle.second);
			glm::vec3 fin = glm::vec3(model * glm::vec4(CurrPerp_RotAngle.first, 1.0f));

			TubeVertices.push_back(fin.x);
			TubeVertices.push_back(fin.y);
			TubeVertices.push_back(fin.z);
			//normal
			TubeVertices.push_back(CurrPerp_RotAngle.first.x);
			TubeVertices.push_back(CurrPerp_RotAngle.first.y);
			TubeVertices.push_back(CurrPerp_RotAngle.first.z);
			//tex
			TubeVertices.push_back(texCoords[texCoordCounter][0]);
			TubeVertices.push_back(texCoords[texCoordCounter][1]);

			//----will be changed
			if (texCoordCounter == 3)
				texCoordCounter = 0;
			else
				texCoordCounter++;

			angleOffset += angleOffsetInc;
		}
	}

	//untill indicecounter2 == tubevertices.size()/8 se we go over all the vertices;
	//since we are zigzaging for triangle strip
	while (indiceCounter2 != TubeVertices.size() / 8)
	{
		if (TubeIndices.size() % 2 == 0)
			TubeIndices.push_back(indiceCounter1++);
		else
			TubeIndices.push_back(indiceCounter2++);
	}

	vertices = TubeVertices;
	indices = TubeIndices;
}



#endif