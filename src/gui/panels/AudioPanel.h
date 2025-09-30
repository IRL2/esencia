#pragma once

#include "EsenciaPanelBase.h"

class AudioPanel : public EsenciaPanelBase {

	// default initial values

	const int WIDTH = 200;
	const int HEIGHT = 30;

	const ofRectangle PANEL_RECT = ofRectangle(35, 15, 8, 8);
	const ofColor BG_COLOR = ofColor(100, 200, 100, 100);

	ofParameter<float> avgClusterSize;
	ofParameter<float> collisionRate;
	ofParameter<float> clusterSizeRate;

	SonificationParameters* params;
    SimulationParameters* simParams;

public:
	void setup(ofxGui& gui, SonificationParameters* params, SimulationParameters* simParams) {
        this->params = params;
        this->simParams = simParams;

		panel = gui.addPanel("sonification");

		ofxGuiContainer* p = panel->addContainer("",
			ofJson({ {"direction", "horizontal"} }));

		p->add<ofxGuiValuePlotter>(params->collisions.set("collisions", 0, 0, 100), ofJson({ {"precision", 0} }));
		p->add<ofxGuiValuePlotter>(params->clusters.set("clusters", 0, 0, 100), ofJson({ {"precision", 0} }));
		p->add<ofxGuiValuePlotter>(params->clusterParticles.set("particles in clusters", 0, 0, 100), ofJson({ {"precision", 0} }));
		p->add<ofxGuiValuePlotter>(params->avgClusterSize.set("avg cluster particles", 0, 0, 100), ofJson({ {"precision", 1} }));
        p->add<ofxGuiValuePlotter>(params->collisionRate.set("collision rate", 0, 0, 1), ofJson({ {"precision", 3} }));
        p->add<ofxGuiValuePlotter>(params->clusterSizeRate.set("cluster size rate", 0, 0, 1), ofJson({ {"precision", 3} }));

		ofAddListener(ofEvents().update, this, &AudioPanel::update);

		configVisuals(PANEL_RECT, BG_COLOR);
	}

	void update(ofEventArgs&)
	{
        //avgClusterSize.set(params->clusters == 0 ? 0 : (float)params->clusterParticles / (float)params->clusters);
		
        //collisionRate.set(params->clusters == 0 ? 0 : (float)params->collisions / (float)simParams->amount);

        //clusterSizeRate.set(params->clusters == 0 ? 0 : (float)params->clusterParticles / (float)simParams->amount);

        // calculate avgParticlesPerCluster
	}

};

