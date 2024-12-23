#pragma once

#include "ofMain.h"
#include "ofxGuiExtended.h"
#include "ofxOpenCv.h"

#include "parameters/SimulationParameters.h"
#include "parameters/RenderParameters.h"
#include "parameters/CameraParameters.h"


#include "panels/ParticlesPanel.h"
#include "panels/SystemstatsPanel.h"
#include "panels/SimulationPanel.h"
#include "panels/RenderPanel.h"
#include "panels/VideoOriginPanel.h"
#include "panels/VideoProcessingPanel.h"


//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true


const float PARTICLES_MIN = 1.0;
const float PARTICLES_MAX = 200.0;


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

private:
    ParticlesPanel particlesPanel;
        
    SystemstatsPanel systemstatsPanel;
        
    SimulationPanel simulationPanel;
        
    VideoProcessingPanel videoProcessingPanel;
        
    VideoOriginPanel videoOriginPanel;
        
    RenderPanel renderPanel;
     

    void drawLineBetween(EsenciaPanelBase &a, EsenciaPanelBase&b);

    ofFbo fbo;  // for the extra layers behind the GUI (lines, background, etc)
};












