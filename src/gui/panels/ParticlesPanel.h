#pragma once

#include "EsenciaPanelBase.h"
#include "ParticleSizeSlider.h"

class ParticlesPanel : public EsenciaPanelBase {

	// default initial values
	const float PARTICLES_INITIAL = 100;
	const float PARTICLES_MIN = 1.0;
	const float PARTICLES_MAX = 5000.0;

	const int RADIUS_INITIAL = 3;
	const int RADIUS_MIN = 1;
	const int RADIUS_MAX = 30;

	const int CIRCULAR_WIDTH = 180;
	const int CIRCULAR_HEIGHT = 180;
	const int RADIUS_HEIGHT = 50;

	 //exponential slider consts
	const int RESOLUTION = 100;
	const float E = exp(1);


	const ofRectangle PANEL_RECT = ofRectangle(1, 1, 8, 0);


	// functions for a functionSlider, an inverse expo slider
	// thanks to angelTC for the (rev)inverse exponential functions
	float linear(float x) {
		return x * 10;
	}

	float reverseLinear(float y) {
		return y / 10;
	}

	float exponentialFunction(float x) {
		return pow(10, x);
	}

	float reversedExponentialFunction(float y) {
		return log10(y);
	}

	std::function<float(float)> inverseExponentialFunction = [this](float x) {
		float a = log((RESOLUTION * E) + 1);
		float b = PARTICLES_MAX / a;
		float c = log((E * x * RESOLUTION) + 1);
		float d = b * c;
		return d;
		};

	std::function<float(float)> reversedInverseExponentialFunction = [this](float y) {
		float a = y / PARTICLES_MAX;
		float b = log((RESOLUTION * E) + 1);
		float c = 1 / E;
		float d = a * b;
		float e = exp(d - 1) - c;
		return e / RESOLUTION;
		};


public:
	void setup(ofxGui &gui, SimulationParameters &params) {
		panel = gui.addPanel("particles");
		
		// this one uses exponential sliders
		//ofxGuiFloatFunctionSlider* functionAmmount = panel->add<ofxGuiFloatFunctionSlider>(
		//	params.amount.set("amount", 120, PARTICLES_MIN, PARTICLES_MAX) , 
		//	ofJson({{"type", "circular"}, {"width", 180}, {"height", 130}, {"precision", 0}}) );
		//functionAmmount->setFunctions(inverseExponentialFunction, reversedInverseExponentialFunction);
		
		// switching back to regular slider, since it does not refresh the visuals when updating the parameter directly (that is crucial for the state and interpolation control)
		panel->add(params.amount.set("amount", 
			PARTICLES_INITIAL, PARTICLES_MIN, PARTICLES_MAX), 
			ofJson({ {"type", "circular"}, {"width", CIRCULAR_WIDTH}, {"height", CIRCULAR_HEIGHT}, {"precision", 0} }));
		
		panel->add<ParticleSizeSlider>(params.radius.set("size",
			RADIUS_INITIAL, RADIUS_MIN, RADIUS_MAX),
			ofJson({ {"height", RADIUS_HEIGHT}, {"precision", 0} }));


		configVisuals(PANEL_RECT, ofColor(200, 20, 20, 100));
	}

};