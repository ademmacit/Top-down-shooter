#ifndef LOGGER_H
#define LOGGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#include <algorithm> //for std::min
#include "TextRender.h"


struct LogEntry
{
	float time;
	std::string text;
	std::string sourceFile;
	glm::vec3 color;

	std::string fullText;
	LogEntry(float Time,std::string Text,std::string SourceFile,glm::vec3 Color)
	{
		time = Time;
		text = Text;
		sourceFile = SourceFile;
		color = Color;
		buildFullText();
	}
	void buildFullText()
	{
		float currTime = (float)glfwGetTime();
		std::string outText;
		outText.append(std::to_string(time));
		outText.append(" | ");
		outText.append(sourceFile);
		outText.append(" - ");
		outText.append(text);
		fullText = outText;
	}

	//copy constructor
	LogEntry(const LogEntry& other):
		time(other.time), text(other.text), sourceFile(other.sourceFile),
		color(other.color),fullText(other.fullText)
	{
		std::cout << "A LOGENTRY COPIED "<< "\n";

	}

};

class Logger
{
public:
	Logger();
	void add(const std::string& InText, const std::string& FileName, const glm::vec3& Color = { 0.8, 0.5, 0.0f })
	{
		float currTime = (float)glfwGetTime();
		mainLog.emplace_back(currTime, InText, FileName, Color);
	}
	std::vector<LogEntry> &get_logs()
	{
		//to avoid copy we return ref
		return mainLog;
	}
	void writeToConsole()
	{
		system("CLS");
		if (mainLog.size() > 50)
		{
			std::cout << "**********second half of the log**********" << std::endl;
			for (size_t i = mainLog.size() / 2; i < mainLog.size(); i++)
				std::cout << mainLog[i].fullText << std::endl;
		}
		else
		{
			for (size_t i = 0; i < mainLog.size(); i++)
				std::cout << mainLog[i].fullText << std::endl;
		}

	}
	// this is extremely inefficeint only use for debugging
	void writeToScreen(TextRenderHandler* TextHandler, Shader* TextShader,const int &scrHeight)
	{
		//max 25 rows for 0.5 scale , -25 ofset for loop , 720 heigh
		// 2.0f height for each letter * 0.5 scale = 1.0f
		// 700 / 25 = 28 , so around 25

		float yOffSet = (float)scrHeight - 25.0f;
		size_t startIndex = 0;

		// if its overflowing from screen write last n ones. n is calculated with scrHeight according to default text values these are 0.5 scale and 25.0f padding
		if (mainLog.size()>(unsigned)(scrHeight / 26))
			startIndex = mainLog.size() - (scrHeight / 26);

		for (size_t i = startIndex; i < mainLog.size(); i++)
		{
			TextHandler->renderText(TextShader,mainLog[i].fullText, 0, yOffSet, 0.5f, mainLog[i].color);
			yOffSet -= 25;
		}
	}
private:
	std::vector<LogEntry> mainLog;
};
Logger::Logger()
{
}


#endif
