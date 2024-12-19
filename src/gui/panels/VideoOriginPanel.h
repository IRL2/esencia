#pragma once

#include "EsenciaPanelBase.h"
#include "parameters/SimulationParameters.h"

class VideoOriginPanel : public EsenciaPanelBase {

    const bool SOURCE_ORBBEC = { false };
    const bool SOURCE_FILE = { false };

    const int CLIP_NEAR_INITIAL = 20;
    const int CLIP_NEAR_MIN = 0;
    const int CLIP_NEAR_MAX = 255;
    const int CLIP_FAR_INITIAL = 170;

    const ofRectangle PANEL_RECT = ofRectangle(1, 11, 8, 0);
    const ofColor BG_COLOR = ofColor(30, 30, 200, 100);

public:
	void setup(ofxGui &gui, CameraParameters &params) {
        panel = gui.addPanel("video origin");

        // SOURCES
        ofxGuiGroup* cameraSourcePanel = panel->addGroup("sources");
        cameraSourcePanel->add<ofxGuiGraphics>("source", &params.previewSource.getTexture(), 
            ofJson({ {"height", 200} }));

        cameraSourcePanel->add(params._sourceOrbbec.set("orbbec camera", SOURCE_ORBBEC));

        cameraSourcePanel->add(params._sourceVideofile.set("video file", SOURCE_FILE));
        //cameraSourcePanel->setWidth(w * 30);
        cameraSourcePanel->minimize();

        // DEPTH CLIPPING
        ofxGuiGroup* cameraClippingPanel = panel->addGroup("depth clipping");
        
        params.clipNear.set("v", 
            CLIP_NEAR_INITIAL, CLIP_NEAR_MIN, CLIP_NEAR_MAX);
        
        //params.clipFar.set("f", 
        //    CLIP_FAR_INITIAL, CLIP_NEAR_MIN, CLIP_NEAR_MAX);
        params.clipFar.set(CLIP_FAR_INITIAL);

        cameraClippingPanel->add<ofxGuiIntRangeSlider>(params.clipNear, params.clipFar);

        //cameraClippingPanel->add(params.clipNear);
        //cameraClippingPanel->add(params.clipFar);


        //cameraClippingPanel->add<ofxGuiIntRangeSlider>(params.clipNear, params.clipFar,
        //    ofJson({ {"precision", 2}, {"direction", "horizontal"}, {"width", 8*30 } }));

        //cameraClippingPanel->setWidth(8 * 30);
        //cameraClippingPanel->minimize();

        // BACKGROUND
        ofxGuiGroup* cameraBackgroundPanel = panel->addGroup("background");
        cameraBackgroundPanel->add<ofxGuiGraphics>("reference", &params.previewBackground.getTexture(), 
            ofJson({ {"height", 200} }));

        cameraBackgroundPanel->add<ofxGuiButton>(params.startBackgroundReference.set("save reference frame", false), 
            ofJson({ {"type", "fullsize"}, {"text-align", "center"} }));

        //cameraBackgroundPanel->setWidth(w * 30);
        cameraBackgroundPanel->minimize();

        configVisuals(PANEL_RECT, BG_COLOR);
    }

};