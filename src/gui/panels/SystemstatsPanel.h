#pragma once

#include "EsenciaPanelBase.h"
#include "parameters/SimulationParameters.h"

class SystemstatsPanel : public EsenciaPanelBase {

	// default initial values
	
	const bool LIMIT_FPS = { true };

	const ofRectangle PANEL_RECT = ofRectangle(12, 1, 8, 0);

	const int WIDTH = 200;

public:
	void setup(ofxGui &gui, SimulationParameters params) {
		panel = gui.addPanel("performance");

		ofxGuiContainer* p = panel->addContainer("", 
			ofJson({ {"direction", "horizontal"} }));

		p->addFpsPlotter(ofJson({ {"width", WIDTH} }));

		p->add(params.lowFps.set("30fps", LIMIT_FPS),
			ofJson({ {"type", "radio"} }));

		params.lowFps.addListener(this, &SystemstatsPanel::limitFps); // TODO:: listeners back to the guiapp x_x

		configVisuals(PANEL_RECT);

	}

	void limitFps(bool& v)
	{
		if (v) {
			ofSetFrameRate(30);
		}
		else {
			ofSetFrameRate(60);
		}
	}

};


