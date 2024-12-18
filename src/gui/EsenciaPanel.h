#pragma once

#include "ofxGuiExtended.h"
#include "Gui.h"

class EsenciaPanel {
public:
	ofxGuiPanel* panel;
	ofxGui& gui;

	std::vector<ParametersBase*> parametersList;

	void setup(ofxGui& gui, ParametersBase* parameters) {
        parametersList.push_back(parameters);
	}

    void printParameters() {
        for (auto* params : parametersList) {
            ofLogNotice() << params->getParameters().getName();
            for (auto& param : params->getParameters()) {
                ofLogNotice() << param->getName();
            }
        }
    }
};
