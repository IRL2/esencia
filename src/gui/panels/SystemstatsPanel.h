#pragma once

#include "EsenciaPanelBase.h"
#include "SystemUsage.h"

class SystemstatsPanel : public EsenciaPanelBase {

	// default initial values
	
	const int WIDTH = EsenciaPanelBase::SIZE_UNIT * 6;
	const int HEIGHT = EsenciaPanelBase::SIZE_UNIT;

	const ofRectangle PANEL_RECT = ofRectangle(13, 1, 7, 0);
	const ofColor BG_COLOR = ofColor(100, 100, 100, 100);

	SystemUsage systemUsage;
	ofParameter<float> cpuUsage;

public:
	void setup(ofxGui &gui, SimulationParameters &params) {
		panel = gui.addPanel("performance");

		ofxGuiContainer* p = panel->addContainer("", 
			ofJson({ {"direction", "horizontal"} }));

		p->addFpsPlotter(ofJson({ {"width", WIDTH}, {"height", HEIGHT} } ));

		ofSetFrameRate(90);

		systemUsage.setup();
		p->add<ofxGuiValuePlotter>(cpuUsage.set("% CPU", 0, 0, 100), ofJson({ {"precision", 0} }));
		ofAddListener(ofEvents().update, this, &SystemstatsPanel::update);

		configVisuals(PANEL_RECT, BG_COLOR);
	}

	void update(ofEventArgs&)
	{
		cpuUsage = systemUsage.getSmoothedCPUUsage();
		systemUsage.update();
	}

};

