#pragma once

#include "ofMain.h"
#include "../gui/Gui.h"
#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here


class Simulator
{
public:
	// lifecycle
	void setup(Gui::SimulationParameters* params, Gui* globalParams);
	void update();

	// listeners
	void updateWorldSize(int _width, int _height);
	void onRenderwindowResize(glm::vec2& worldSize);
	void onGUIChangeAmmount(int& value);

	void recieveFrame(ofxCvGrayscaleImage frame);

	// properties
	vector<glm::vec4> particles;

	// environment
	int width = 700;
	int height = 600;

	// parameters
	Gui::SimulationParameters* parameters;
	Gui* globalParameters;

private:
	void generateParticles(int ammount);

};


