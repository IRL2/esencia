#pragma once

#include "ofxGuiExtended.h"


class EsenciaPanelBase {
public:
	ofxGuiPanel* panel;

    void configVisuals(ofRectangle rect, ofColor color) {
        panel->loadTheme("support/gui-styles.json", true);
        panel->setBackgroundColor(color);

        panel->setPosition(rect.x * 30, rect.y * 30);
        panel->setWidth(rect.width * 30);
        //panel->setHeight(rect.height * 30);
    }

 //   void printParameters() {
 //       for (auto* params : parametersList) {
 //           ofLogNotice() << params->getParameters().getName();
 //           for (auto& param : params->getParameters()) {
 //               ofLogNotice() << param->getName();
 //           }
 //       }
 //   }
};
