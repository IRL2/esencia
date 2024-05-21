#pragma once

#include "ofMain.h"
#include "ofxGui.h"

//#define DEBUG_IMAGES true
//#define RECORD_TESTING_VIDEO true

class Gui
{
public:
    void setup();
    void update();
    void draw();

    ofxPanel guiPanel;

    // ofxPanel simulationPanel;
    // ofxPanel renderPanel;

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
        ofParameter<int> ammount;
        ofParameter<int> radius;
        ofParameter<float> targetTemperature;
        ofParameter<float> coupling;
        ofParameter<bool> applyThermostat;
    };
    SimulationParameters simulationParameters;

    struct RenderParameters
    {
        ofParameter<int> size;
        ofParameter<ofColor> color;
        ofParameter<bool> useShaders;
        ofParameter<bool> useFaketrails;
    };
    RenderParameters renderParameters;


    struct CameraParameters
    {
        ofParameter<bool> enableClipping;
        ofParameter<int> clipFar; // threshold low
        ofParameter<int> clipNear; // threshold hi

        ofParameter<float> blobMinArea;
        ofParameter<float> blobMaxArea;
        ofParameter<int> gaussianBlur;
        ofParameter<int> nConsidered;
        ofParameter<bool> fillHolesOnPolygons;
        ofParameter<bool> floodfillHoles;

        ofParameter<float> polygonTolerance = 2.0f;
        ofParameter<bool> showPolygons;

        ofParameter<bool> startBackgroundReference;
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
