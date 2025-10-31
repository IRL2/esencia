#pragma once

#include "EsenciaPanelBase.h"

class AudioPanel : public EsenciaPanelBase {

	// default initial values

	const ofRectangle PANEL_RECT = ofRectangle(37, 15, 8, 8);
	const ofColor BG_COLOR = ofColor(100, 200, 100, 100);

	ofParameter<float> avgClusterSize;
	ofParameter<float> collisionRate;
	ofParameter<float> particlesInClusterRate;

	SonificationParameters* params;
    SimulationParameters* simParams;

public:
	void setup(ofxGui& gui, SonificationParameters* params, SimulationParameters* simParams) {
        this->params = params;
        this->simParams = simParams;

		panel = gui.addPanel("sonification analysis");

		ofxGuiContainer* p = panel->addContainer("",
			ofJson({ {"direction", "vertical"} }));

		ofxGuiGroup* simGroup = p->addGroup("sonification data");
		simGroup->add<ofxGuiValuePlotter>(params->collisions.set("collisions", 0, 0, 100), ofJson({ {"precision", 0} }));
        simGroup->add<ofxGuiValuePlotter>(params->collisionRate.set("collision rate", 0, 0, 1), ofJson({ {"precision", 2} }));
		
		simGroup->add<ofxGuiValuePlotter>(params->clusters.set("clusters", 0, 0, 100), ofJson({ {"precision", 0} }));
		//simGroup->add<ofxGuiValuePlotter>(params->particlesInClusters.set("particles in clusters", 0, 0, 100), ofJson({ {"precision", 0} }));
		//simGroup->add<ofxGuiValuePlotter>(params->avgClusterSize.set("avg cluster size", 0, 0, 100), ofJson({ {"precision", 1} }));
        simGroup->add<ofxGuiValuePlotter>(params->particlesInClusterRate.set("particles in clusters %", 0, 0, 1), ofJson({ {"precision", 2} }));

        simGroup->add<ofxGuiValuePlotter>(params->avgClusterVelocity.set("avg cluster velocity", 0, 0, 10), ofJson({ {"precision", 2} }));
        simGroup->add<ofxGuiValuePlotter>(params->avgClusterSpatialSpread.set("avg cluster spread", 0, 0, 100), ofJson({ {"precision", 2} }));
        simGroup->add<ofxGuiValuePlotter>(params->avgClusterVelocityMagitude.set("avg cluster vel mag", 0, 0, 10), ofJson({ {"precision", 2} }));

        ofxGuiGroup* volumeGroup = p->addGroup("volumes", 
			ofJson({ {"direction", "horizontal"} }));
		volumeGroup->add(params->masterVolume.set("master", 0.8, 0.0, 1.2), ofJson({ {"precision", 1} }));
		volumeGroup->add(params->collisionVolume.set("collisions", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));
		volumeGroup->add(params->clusterVolume.set("clusters", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));
		volumeGroup->add(params->backgroundVolume.set("background", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));

  //      p->add(params->masterVolume.set("master volume", 0.8, 0.0, 1.2), ofJson({ {"precision", 1} }));
		//p->add(params->polysynthVolume.set("cluster volume", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));
		//p->add(params->sampler1playerVolume.set("sampler1 volume", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));
		//p->add(params->sampler2playerVolume.set("sampler2 volume", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));
  //      p->add(params->datasynthVolume.set("datasynth volume", 0.8, 0.0, 1.0), ofJson({ {"precision", 1} }));

		//p->add(params->eqTrebble.set("eq trebble", 0.5, 0.0, 1.0), ofJson({ {"precision", 2} }));
        //p->add(params->eqBass.set("eq bass", 0.5, 0.0, 1.0), ofJson({ {"precision", 2} }));

		ofAddListener(ofEvents().update, this, &AudioPanel::update);

		configVisuals(PANEL_RECT, BG_COLOR);
	}

	void update(ofEventArgs&)
	{

	}

};

