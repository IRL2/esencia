#pragma once

#include "ofMain.h"
#include "ofxGuiExtended.h"
#include "ofxOpenCv.h"

#include "ofxPresetsParametersBase.h"
#include "EsenciaParameters.h"

#include "panels/ParticlesPanel.h"
#include "panels/SystemstatsPanel.h"
#include "panels/SimulationPanel.h"
#include "panels/RenderPanel.h"
#include "panels/VideoOriginPanel.h"
#include "panels/VideoProcessingPanel.h"
#include "panels/PresetsPanel.h"
#include "panels/SequencePanel.h"
#include "panels/AudioPanel.h"


#include "ofxPresets.h"


//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true



class GuiApp
{
public:
    void setup();
    void update();
    void draw();

    void keyReleased(ofKeyEventArgs& e);
    void windowResized(int w, int h);

    ofxGui gui;
    
    // structs to separate different engine values (and allow name duplication without extra verbosity) 
    // so each groups/struct can be assigned to its specific system

    SimulationParameters simulationParameters;

    RenderParameters renderParameters;

    CameraParameters cameraParameters;

    PresetsParameters presetsParameters;

    SonificationParameters sonificationParameters;

private:
    ParticlesPanel particlesPanel;
        
    SystemstatsPanel systemstatsPanel;
        
    SimulationPanel simulationPanel;
        
    VideoProcessingPanel videoProcessingPanel;
        
    VideoOriginPanel videoOriginPanel;
        
    RenderPanel renderPanel;

    PresetsPanel presetsPanel;

	SequencePanel sequencePanel;

    AudioPanel audioPanel;
     

    ofFbo fbo;  // for the extra layers behind the GUI (lines, background, etc)

    // for the presets and their needed parameter refs
    std::vector<ofxPresetsParametersBase*> allParameters;

    ofxPresets presetManager;

    ofColor bgColor1 = ofColor::darkSlateGray;
	ofColor bgColor2 = ofColor::darkSalmon;
    ofColor bgTargetColor1 = bgColor2;
    ofColor bgStartColor1  = bgColor1;
    ofColor bgTargetColor2 = bgColor1;
    ofColor bgStartColor2  = bgColor2;
    float bgChangeFrequency = 0;
    int bgChangeDuration = 1000;
    std::vector<ofColor> bgColors = { ofColor::darkSalmon, ofColor::darkGrey, ofColor::darkSlateGray, ofColor::darkKhaki, ofColor::darkViolet, ofColor::deepSkyBlue };
};












