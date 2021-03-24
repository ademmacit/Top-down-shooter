#ifndef TextRender_H
#define TextRender_H
/*
Due to how FreeType is developed (at least at the time of this writing), you cannot put their header files in
a new directory; they should be located at the root of your include directories. Including FreeType like #include
<FreeType/ft2build.h> will likely cause several header conflicts.
 */
#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include "ShaderC.h"
#include "Globals.h"


struct Character
{
	GLuint TextureID; //ID handle of the glyph texture
	glm::ivec2 Size; //size of the glyph
	glm::ivec2 Bearing; //offset from baseline to top/left of glyph
	GLuint Advance; //offset to advance to next glyph
};


class TextRenderHandler
{
public:
	TextRenderHandler();

	void renderText(Shader *activeShader,std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color)
	{
		glEnable(GL_CULL_FACE);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH_TEST);

		activeShader->setVec3("textColor", color);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(VAO);
		// Iterate through all characters 
		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];

			GLfloat xpos = x + ch.Bearing.x * scale;
			GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;


			GLfloat w = ch.Size.x * scale;
			GLfloat h = ch.Size.y * scale;
			//update VBO for each character
			GLfloat vertices[] =
			{ 
				 xpos,     ypos + h, 0.0f,		 0.0, 0.0 ,
				 xpos,     ypos,     0.0f,		 0.0, 1.0 ,
				 xpos + w, ypos,     0.0f,		 1.0, 1.0 ,

				 xpos,     ypos + h, 0.0f,		 0.0, 0.0 ,
				 xpos + w, ypos,     0.0f,		 1.0, 1.0 ,
				 xpos + w, ypos + h, 0.0f,		 1.0, 0.0
			};
			//render glyph texture over quad
			glBindTexture(GL_TEXTURE_2D, ch.TextureID);

			//update content of the VBO memory
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			//render
			glDrawArrays(GL_TRIANGLES, 0, 6);
			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)      
			x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray(0);    
		glBindTexture(GL_TEXTURE_2D, 0);

		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);

		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	}

	//*initialises text rendering. Loads chars
	//since this needs to be done after opengl initialisations textrendering initialisations 
	//are moved here so we can use this class as a property of another class
	void init()
	{

		if (FT_Init_FreeType(&ft))
			std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

		if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face))
			std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;

		//*************filling the characters map*************

		FT_Set_Pixel_Sizes(face, width, height);//set size to load glyphs

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);//disable byte-aligment restriction

		//loading 128 ascii chars
		for (GLubyte c = 0; c < 128; c++)
		{
			if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			//generate texture
			GLuint texture;
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RED,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_RED,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer);
			//set texture options
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			//storing character
			Character character = {
				texture,
				glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),
				glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),
				(GLuint)face->glyph->advance.x
			};
			Characters.insert(std::pair<GLchar, Character>(c, character));
		}
		glBindTexture(GL_TEXTURE_2D, 0); //we bind it again while drawing

		//clearing freetype after we are done with it
		FT_Done_Face(face);
		FT_Done_FreeType(ft);

		// Configure VAO/VBO for texture quads  
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 5 * 6, NULL, GL_DYNAMIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

private:

	FT_Library ft;
	FT_Face face;
	//0 allows for dynamic width according to height
	int width = 0;
	int height = 48;

	GLuint VAO, VBO;

	std::map<GLchar, Character> Characters;
};

TextRenderHandler::TextRenderHandler()
{
	
	
}



#endif