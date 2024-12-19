#pragma once

#include "EsenciaPanelBase.h"
#include "parameters/SimulationParameters.h"

class ParticlesPanel : public EsenciaPanelBase {

	// default initial values
	const float PARTICLES_INITIAL = 120;
	const float PARTICLES_MIN = 1.0;
	const float PARTICLES_MAX = 200.0;

	const int RADIUS_INITIAL = 3;
	const int RADIUS_MIN = 1;
	const int RADIUS_MAX = 30;

	const int CIRCULAR_WIDTH = 180;
	const int CIRCULAR_HEIGHT = 180;
	const int RADIUS_HEIGHT = 50;

	const ofRectangle PANEL_RECT = ofRectangle(1, 1, 8, 0);


public:
	void setup(ofxGui &gui, SimulationParameters &params) {
		panel = gui.addPanel("particles");
		
		// this one uses exponential sliders
		//ofxGuiFloatFunctionSlider* functionAmmount = particlesPanel->add<ofxGuiFloatFunctionSlider>(simulationParameters.amount.set("amount", 120, PARTICLES_MIN, PARTICLES_MAX) , ofJson({{"type", "circular"}, {"width", 180}, {"height", 130}, {"precision", 0}}) );
		//functionAmmount->setFunctions(inverseExponentialFunction, reversedInverseExponentialFunction);		
		// switching back to regular slider, since it does not refresh the visuals when updating the parameter directly (that is crucial for the state and interpolation control)
		panel->add(params.amount.set("amount", 
			PARTICLES_INITIAL, PARTICLES_MIN, PARTICLES_MAX), 
			ofJson({ {"type", "circular"}, {"width", CIRCULAR_WIDTH}, {"height", CIRCULAR_HEIGHT}, {"precision", 0} }));
		
		panel->add(params.radius.set("scale", 
			RADIUS_INITIAL, RADIUS_MIN, RADIUS_MAX),
			ofJson({{"height", RADIUS_HEIGHT}, {"precision", 0}}) );

		configVisuals(PANEL_RECT, ofColor(200, 20, 20, 100));
	}

};