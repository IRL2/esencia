#pragma once

#include "EsenciaPanelBase.h"

class RenderPanel : public EsenciaPanelBase {

    // default initial values
    const bool USE_FAKETRAILS = { false };
    const bool USE_SHADERS = { true };
    const ofColor PARTICLE_COLOR = ofColor(77, 130, 200);
    
    const float VIDEO_VISIBILITY_INITIAL = 0.3;
    const float VIDEO_VISIBILITY_MIN = 0.0;
    const float VIDEO_VISIBILITY_MAX = 1.0;

    const bool VIDEO_PREVIEW = { false };

    const float TRAILS_VISIBILITY_INITIAL = 0.7; // 0.95 // 0.95
    const float TRAILS_VISIBILITY_MIN = 0.0; // 1 // 0.0
    const float TRAILS_VISIBILITY_MAX = 0.7; // 0.7 // 0.3

    const ofColor VIDEO_COLOR = ofColor::white;

    const ofRectangle PANEL_RECT = ofRectangle(25, 12, 8, 0);
    const ofColor& BG_COLOR = ofColor(100, 20, 100, 100);


public:
	void setup(ofxGui &gui, RenderParameters &params) {
		panel = gui.addPanel("render");

        panel->add(params.color.set("particle color", 
            PARTICLE_COLOR)
        )->minimize();

        //panel->add(params.useShaders.set("use shaders",  USE_SHADERS));

        // video
        ofxGuiGroup* vp = panel->addGroup("video");
        //vp->add(params.showVideoPreview.set("visible", VIDEO_PREVIEW));
        
        vp->add(params.videopreviewVisibility.set("visibility", 
            VIDEO_VISIBILITY_INITIAL, VIDEO_VISIBILITY_MIN, VIDEO_VISIBILITY_MAX),
            ofJson({ {"precision", 3} }));
        
        vp->add(params.videoColor.set("color", VIDEO_COLOR))->minimize();
        
        // ADD FEEDBACK CONTROL
        vp->add(params.useVideoFeedback.set("aura feedback", false));
        
        // NEW: Add warp effect controls
        ofxGuiGroup* warpGroup = vp->addGroup("warp effects");
        warpGroup->add(params.useWarpEffect.set("use warp effect", true));
        warpGroup->add(params.warpVariance.set("variance", 0.02f, 0.0f, 10.1f), ofJson({ {"precision", 3} }));
        warpGroup->add(params.warpPropagation.set("propagation", 0.02f, 0.0f, 0.1f), ofJson({ {"precision", 3} }));
        warpGroup->add(params.warpPropagationPersistence.set("persistence", 0.95f, 0.8f, 1.0f), ofJson({ {"precision", 3} }));
        warpGroup->add(params.warpSpreadX.set("spread X", 1.0f, 0.1f, 5.0f), ofJson({ {"precision", 2} }));
        warpGroup->add(params.warpSpreadY.set("spread Y", 1.0f, 0.1f, 5.0f), ofJson({ {"precision", 2} }));
        warpGroup->add(params.warpDetail.set("detail", 1.0f, 0.1f, 4.0f), ofJson({ {"precision", 2} }));
        warpGroup->add(params.warpBrightPassThreshold.set("brightness", 0.0f, 0.0f, 1.0f), ofJson({ {"precision", 3} }));
        warpGroup->minimize(); // Start collapsed for cleaner interface
        
        // trails
        ofxGuiGroup* ofTrialsPanel = panel->addGroup("trails");

        //ofTrialsPanel->add(params.useFaketrails.set("enable", USE_FAKETRAILS));

        ofTrialsPanel->add(params.fakeTrialsVisibility.set("length", 
            TRAILS_VISIBILITY_INITIAL, TRAILS_VISIBILITY_MIN, TRAILS_VISIBILITY_MAX),
            ofJson({ {"precision", 3} }));

		configVisuals(PANEL_RECT, BG_COLOR);
	}

};