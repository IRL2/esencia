#pragma once

#include "ofMain.h"
#include "ofxGuiExtended.h"
#include "ofxOpenCv.h"


//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true

class Gui
{
public:
    void setup();
    void update();
    void draw();

    void keyReleased(int key);
    void windowResized(int w, int h);

    ofxGui gui;
    
    // structs to separate different engine values (and allow name duplication without extra verbosity) 
    // so each groups/struct can be assigned to its specific system

    struct SimulationParameters
    {
        ofParameter<int> ammount = 10;
        ofParameter<int> radius = 1;
        ofParameter<float> targetTemperature;
        ofParameter<float> coupling;
        ofParameter<bool> applyThermostat;
        ofParameter<glm::vec2> worldSize;
    };
    SimulationParameters simulationParameters;

    struct RenderParameters
    {
        ofParameter<int> size = 3;
        ofParameter<ofColor> color;
        ofParameter<glm::vec2> windowSize; // right now its equals to the render window size
        ofParameter<bool> useShaders = false;
        ofParameter<bool> useFaketrails = false;
    };
    RenderParameters renderParameters;


    struct CameraParameters
    {
        ofParameter<bool> enableClipping = true;
        ofParameter<int> clipFar = 170; // threshold low
        ofParameter<int> clipNear = 20; // threshold hi

        ofParameter<float> blobMinArea = 0.05f;
        ofParameter<float> blobMaxArea = 0.8f;
        ofParameter<int> gaussianBlur = 0;
        ofParameter<int> nConsidered = 0;
        ofParameter<bool> fillHolesOnPolygons = false;
        ofParameter<bool> floodfillHoles = true;

        ofParameter<float> polygonTolerance = 2.0f;
        ofParameter<bool> showPolygons = false;

        ofParameter<bool> startBackgroundReference = true;
        //ofParameter<int> backgroundSamples;
        ofParameter<bool> saveDebugImages = false;
        ofParameter<bool> recordTestingVideo = false;
        ofParameter<bool> useMask = false;

        ofParameter<bool> _sourceOrbbec = false;
        ofParameter<bool> _sourceVideofile = false;
        ofParameter<bool> _sourceWebcam = false;

        ofImage previewSegment;
        ofImage previewSource;
        ofImage previewBackground;
        // ofParameter<ofxGuiGraphics> background;
    };
    CameraParameters cameraParameters;

    private:
        // ofParameterGroup *simulationConfig;

        ofxGuiPanel* particlesPanel;
        ofxGuiPanel* simulationPanel;
        ofxGuiPanel* renderPanel;
        ofxGuiPanel* statsPanel;
        ofxGuiPanel* videoPanel;
        ofxGuiPanel* cameraSourcePanel;
        ofxGuiPanel* cameraClippingPanel;
        ofxGuiPanel* cameraProcessingPanel;
        ofxGuiPanel* cameraPolygonsPanel;
        ofxGuiPanel* cameraBackgroundPanel;

        ofxGuiPanel cameraGroup;

        void configureParticlesPanel();
        void configureSimulationPanel();
        void configureVideoPanel();
        void configureRenderPanel();
        void configureSystemstatsPanel();
        void configurePresetsPanel();
        
        void drawLineBetween(ofxGuiPanel &a, ofxGuiPanel &b);

        // for the extra layers behind the GUI (lines, background, etc)
        ofFbo fbo;

        // to store the preview videos from the camera, they will be displayed inside gui controls
        ofImage cameraSource, cameraSegment, cameraBackground;

};
