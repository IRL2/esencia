#pragma once
#include "ParametersBase.h"


struct SimulationParameters : public ParametersBase {
    ofParameter<float> amount;
    ofParameter<int> radius;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> lowFps;

    ofParameterGroup parameters;

    SimulationParameters() {
        //parameters.setName("Simulation Parameters");
        //parameters.add(amount.set("Ammount", 10));
        //parameters.add(radius.set("Radius", 1));
        parameters.add(targetTemperature.set("Target Temperature", 0.0f));
        parameters.add(coupling.set("Coupling", 0.0f));
        parameters.add(applyThermostat.set("Apply Thermostat", false));
        parameters.add(worldSize.set("World Size", glm::vec2(0.0f, 0.0f)));
        //parameters.add(lowFps.set("Limited FPS", true));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};