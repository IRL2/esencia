#pragma once

#include "ofMain.h"
//#include "ofxGui.h"
#include "ofxGuiExtended.h"

//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true

class Gui
{
public:
    void setup();
    void update();
    void draw();

    ofxGui gui;
    
    ofxGuiPanel* simulationPanel;
    ofxGuiPanel* renderPanel;
    ofxGuiPanel* cameraPanel;

    ofParameterGroup parameters;

    // (sub)groups for each system
    ofParameterGroup simulation;
    ofParameterGroup render;
    ofParameterGroup camera;

    ofParameter<string> guiSeparator;

    // structs to separate different engine values (and allow name duplication without extra verbosity) 
    // so each groups/struct can be assigned to its specific system

    struct SimulationParameters
    {
        ofParameter<int> ammount = 10;
        ofParameter<float> momentum = 4.0f;
        ofParameter<int> radius = 1;
    };
    SimulationParameters simulationParameters;

    struct RenderParameters
    {
        ofParameter<int> size = 3;
        ofParameter<ofColor> color;
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
    };
    CameraParameters cameraParameters;


};
