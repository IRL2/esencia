#pragma once

#include "ofMain.h"
#include "ofxGuiExtended.h"
#include "ofxOpenCv.h"


//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true


const float PARTICLES_MIN = 1.0;
const float PARTICLES_MAX = 200.0;


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
        ofParameter<float> ammount = 10;
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
        ofParameter<bool> showVideoPreview = false;
        ofParameter<float> fakeTrialsVisibility = 0.0; // layer alpha value that produces particle trails
        ofParameter<float> videopreviewVisibility = 0.0; // layer alpha value
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
        ofxGuiPanel* videoOriginPanel;
        ofxGuiPanel* videoProcessPanel;

        // actually sub groups
        ofxGuiPanel* cameraSourcePreview;
        ofxGuiPanel* cameraSourcePanel;
        ofxGuiPanel* cameraClippingPanel;
        ofxGuiPanel* cameraProcessingPanel;
        ofxGuiPanel* cameraPolygonsPanel;
        ofxGuiPanel* cameraBackgroundPanel;

        ofxGuiPanel cameraGroup;

        void configureParticlesPanel(int x, int y, int w, int h);
        void configureSimulationPanel(int x, int y, int w, int h);
        void configureVideoinitialPanel(int x, int y, int w, int h);
        void configureVideoprocessingPanel(int x, int y, int w, int h);
        void configureRenderPanel(int x, int y, int w, int h);
        void configureSystemstatsPanel(int x, int y, int w, int h);
        void configurePresetsPanel(int x, int y, int w, int h);
        
        void drawLineBetween(ofxGuiPanel &a, ofxGuiPanel &b);

        // for the extra layers behind the GUI (lines, background, etc)
        ofFbo fbo;

        // to store the preview videos from the camera, they will be displayed inside gui controls
        ofImage cameraSource, cameraSegment, cameraBackground;
};
