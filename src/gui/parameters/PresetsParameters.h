#pragma once
#include "ParametersBase.h"


struct PresetsParameters : public ParametersBase {

	// declare parameters for 16 buttons to select the state
    ofParameter<bool> states[16];

    // declare parameters for action buttons: save, clear, copyTo
	ofParameter<bool> save;
	ofParameter<bool> clear;
	ofParameter<bool> copyTo;

    ofParameterGroup parameters;

    PresetsParameters() {
        //parameters.setName("Simulation Parameters");
        //parameters.add(amount.set("Ammount", 10));
        //parameters.add(radius.set("Radius", 1));
        //parameters.add(targetTemperature.set("Target Temperature", 0.0f));
        //parameters.add(coupling.set("Coupling", 0.0f));
        //parameters.add(applyThermostat.set("Apply Thermostat", false));
        //parameters.add(worldSize.set("World Size", glm::vec2(0.0f, 0.0f)));
        //parameters.add(lowFps.set("Limited FPS", true));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};