#pragma once

#include "ofxGuiExtended.h"
#include "parameters/ParametersBase.h"

class EsenciaPanelBase {
public:
	ofxGuiPanel* panel;
	//ofxGui& gui;

	//std::vector<ParametersBase*> parametersList;

    //virtual void setup(ofxGui& gui, ParametersBase params, std::string title);

    void configVisuals(ofRectangle rect, ofColor color) {
        panel->loadTheme("support/gui-styles.json", true);
        panel->setBackgroundColor(color);

        panel->setPosition(rect.x * 30, rect.y * 30);
        panel->setWidth(rect.width * 30);
        //panel->setHeight(rect.height * 30);
    }

    //void 

 //   virtual void configureParameters(ParametersBase* params) {
 //        parametersList.push_back(params);
 //        printParameters();
	//}

 //   void printParameters() {
 //       for (auto* params : parametersList) {
 //           ofLogNotice() << params->getParameters().getName();
 //           for (auto& param : params->getParameters()) {
 //               ofLogNotice() << param->getName();
 //           }
 //       }
 //   }
};
