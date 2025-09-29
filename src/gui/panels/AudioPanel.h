#pragma once

#include "EsenciaPanelBase.h"

class AudioPanel : public EsenciaPanelBase {

	// default initial values

	const int WIDTH = 200;
	const int HEIGHT = 30;

	const ofRectangle PANEL_RECT = ofRectangle(35, 15, 8, 8);
	const ofColor BG_COLOR = ofColor(100, 200, 100, 100);

	//ofParameter<int> collisions;  // taken directly from the params, not used
	//ofParameter<int> clusters;
    ofParameter<float> avgParticlesPerCluster;

	SonificationParameters* params;

public:
	void setup(ofxGui& gui, SonificationParameters* params) {
        this->params = params;
		panel = gui.addPanel("sonification");

		ofxGuiContainer* p = panel->addContainer("",
			ofJson({ {"direction", "horizontal"} }));

		p->add<ofxGuiValuePlotter>(params->collisions.set("collisions", 0, 0, 100), ofJson({ {"precision", 0} }));
		p->add<ofxGuiValuePlotter>(params->clusterParticles.set("clusters", 0, 0, 100), ofJson({ {"precision", 0} }));
		p->add<ofxGuiValuePlotter>(avgParticlesPerCluster.set("particles per cluster", 0, 0, 100), ofJson({ {"precision", 1} }));


		ofAddListener(ofEvents().update, this, &AudioPanel::update);

		configVisuals(PANEL_RECT, BG_COLOR);
	}

	void update(ofEventArgs&)
	{
        // calculate avgParticlesPerCluster
	}

};

