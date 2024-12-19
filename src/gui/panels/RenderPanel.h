#pragma once

#include "EsenciaPanelBase.h"
#include "parameters/SimulationParameters.h"

class RenderPanel : public EsenciaPanelBase {

    // default initial values
    const bool USE_FAKETRAILS = { false };
    const bool USE_SHADERS = { true };
    const ofColor PARTICLE_COLOR = ofColor(77, 130, 200);
    
    const float VIDEO_VISIBILITY_INITIAL = 0.3;
    const float VIDEO_VISIBILITY_MIN = 0.0;
    const float VIDEO_VISIBILITY_MAX = 1.0;

    const bool VIDEO_PREVIEW = { false };

    const float TRAILS_VISIBILITY_INITIAL = 0.95; // 0.95 // 0.95
    const float TRAILS_VISIBILITY_MIN = 0.7; // 1 // 0.0
    const float TRAILS_VISIBILITY_MAX = 1.0; // 0.7 // 0.3

    const ofColor VIDEO_COLOR = ofColor::white;

    const ofRectangle PANEL_RECT = ofRectangle(25, 12, 8, 0);
    const ofColor& BG_COLOR = ofColor(100, 20, 100, 100);


public:
	void setup(ofxGui &gui, RenderParameters params) {
		panel = gui.addPanel("render");

        panel->add(params.color.set("particle color", 
            PARTICLE_COLOR)
        )->minimize();

        panel->add(params.useShaders.set("use shaders", 
            USE_SHADERS));

        // video
        ofxGuiGroup* vp = panel->addGroup("video");
        vp->add(params.showVideoPreview.set("visible", VIDEO_PREVIEW));
        
        vp->add(params.videopreviewVisibility.set("visibility", 
            VIDEO_VISIBILITY_INITIAL, VIDEO_VISIBILITY_MIN, VIDEO_VISIBILITY_MAX),
            ofJson({ {"precision", 3} }));
        
        vp->add(params.videoColor.set("color", VIDEO_COLOR))->minimize();

        // trails
        ofxGuiGroup* ofTrialsPanel = panel->addGroup("trails");

        ofTrialsPanel->add(params.useFaketrails.set("enable", 
            USE_FAKETRAILS));

        ofTrialsPanel->add(params.fakeTrialsVisibility.set("lenght", 
            TRAILS_VISIBILITY_INITIAL, TRAILS_VISIBILITY_MIN, TRAILS_VISIBILITY_MAX),
            ofJson({ {"precision", 3} }));

		configVisuals(PANEL_RECT, BG_COLOR);
	}

};