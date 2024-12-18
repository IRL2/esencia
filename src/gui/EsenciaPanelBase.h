#pragma once

#include "ofxGuiExtended.h"
#include "parameters/ParametersBase.h"

class EsenciaPanelBase {
public:
	ofxGuiPanel* panel;
	//ofxGui& gui;

	std::vector<ParametersBase*> parametersList;

    void setup(ofxGui& gui, std::string name, ofRectangle rect, ParametersBase* params) {
        panel = gui.addPanel(name);

        //panel->setBackgroundColor(ofColor(180, 180, 180, 100));
        panel->loadTheme("support/gui-styles.json", true);

        configureParameters(params);

        panel->setPosition(rect.x * 30, rect.y * 30);
        panel->setWidth(rect.width * 30);
        panel->setHeight(rect.height * 30);
    }

    virtual void configureParameters(ParametersBase* params) {
         parametersList.push_back(params);
         printParameters();
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
