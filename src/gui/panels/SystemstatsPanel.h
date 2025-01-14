#pragma once

#include "EsenciaPanelBase.h"

class SystemstatsPanel : public EsenciaPanelBase {

	// default initial values
	
	bool LIMIT_FPS = { false };

#ifdef DEBUG
	LIMIT_FPS = { true };
#endif

	const int WIDTH = 200;
	const int HEIGHT = 30;

	const ofRectangle PANEL_RECT = ofRectangle(12, 1, 8, 0);
	const ofColor BG_COLOR = ofColor(100, 100, 100, 100);


public:
	void setup(ofxGui &gui, SimulationParameters &params) {
		panel = gui.addPanel("performance");

		ofxGuiContainer* p = panel->addContainer("", 
			ofJson({ {"direction", "horizontal"} }));

		p->addFpsPlotter(ofJson({ {"width", WIDTH}, {"height", HEIGHT} } ));

		p->add(params.lowFps.set("30fps", LIMIT_FPS),
			ofJson({ {"type", "radio"} }));

		limitFps(LIMIT_FPS);

		params.lowFps.addListener(this, &SystemstatsPanel::limitFps); // TODO:: listeners back to the guiapp x_x

		configVisuals(PANEL_RECT, BG_COLOR);

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


