#pragma once

#include "EsenciaPanelBase.h"
#include "parameters/SimulationParameters.h"

class SimulationPanel : public EsenciaPanelBase {

	// default initial values

	const bool APPLY_THERMOSTAT = { true };

	const float TEMPERATURE_INIT = 25000;
	const float TEMPERATURE_MIN  = 1000;
	const float TEMPERATURE_MAX  = 100000;

	const float THERMOSTAT_INIT = 0.5;
	const float THERMOSTAT_MIN  = 0.1;
	const float THERMOSTAT_MAX  = 1.0;

	const float SLIDERS_WIDTH  = 105;
	const float SLIDERS_HEIGHT = 160;

	const ofRectangle PANEL_RECT = ofRectangle(25, 3, 8, 0);


public:
	void setup(ofxGui &gui, SimulationParameters &params) {
		panel = gui.addPanel("simulation");

		panel->add(params.applyThermostat.set("apply thermostat", 
			APPLY_THERMOSTAT));

		ofxGuiContainer* p = panel->addContainer("", 
			ofJson({ {"direction", "horizontal"} }));

		p->add(params.targetTemperature.set("\nequilibrium\ntemperature", 
			TEMPERATURE_INIT, TEMPERATURE_MIN, TEMPERATURE_MAX),
			ofJson({ {"width", SLIDERS_WIDTH}, {"height", SLIDERS_HEIGHT}, {"precision", 0} }));

		p->add(params.coupling.set("Berendsen\nthermostat\ncoupling", 
			THERMOSTAT_INIT, THERMOSTAT_MIN, THERMOSTAT_MAX),
			ofJson({ {"width", SLIDERS_WIDTH}, {"height", SLIDERS_HEIGHT}, {"precision", 3} }));

		configVisuals(PANEL_RECT);
	}

};