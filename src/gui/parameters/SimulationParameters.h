#pragma once
#include "ParametersBase.h"


struct SimulationParameters : public ParametersBase {
    ofParameter<float> amount = 10;
    ofParameter<int> radius = 1;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> limitedFps = true;

    ofParameterGroup parameters;

    SimulationParameters() {
        parameters.setName("Simulation Parameters");
        parameters.add(amount.set("Ammount", 10));
        parameters.add(radius.set("Radius", 1));
        parameters.add(targetTemperature.set("Target Temperature", 0.0f));
        parameters.add(coupling.set("Coupling", 0.0f));
        parameters.add(applyThermostat.set("Apply Thermostat", false));
        parameters.add(worldSize.set("World Size", glm::vec2(0.0f, 0.0f)));
        parameters.add(limitedFps.set("Limited FPS", true));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};