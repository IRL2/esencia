#pragma once

#include "ofMain.h"
#include "ofxGuiExtended.h"
#include "ofxOpenCv.h"


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

    void keyReleased(int key);
    void windowResized(int w, int h);

    ofxGui gui;
    
    // structs to separate different engine values (and allow name duplication without extra verbosity) 
    // so each groups/struct can be assigned to its specific system

    struct SimulationParameters
    {
        ofParameter<float> amount = 10;
        ofParameter<int> radius = 1;
        ofParameter<float> targetTemperature;
        ofParameter<float> coupling;
        ofParameter<bool> applyThermostat;
        ofParameter<glm::vec2> worldSize;
        ofParameter<bool> limitedFps = true;
        // ofParameter<int> fps = 30;
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
        ofParameter<ofColor> videoColor;
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

        // listener
        void limiteFps(bool &v);
};


class ParametersBase {
public:
    virtual ~ParametersBase() = default;

    // Optionally provide a method to retrieve the parameter group
    virtual ofParameterGroup& getParameters() = 0;
};






struct SimulationParameters : public ParametersBase {
    ofParameter<float> ammount = 10;
    ofParameter<int> radius = 1;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> limitedFps = true;

    ofParameterGroup parameters;

    SimulationParameters() {
        parameters.setName("Simulation Parameters");
        parameters.add(ammount.set("Ammount", 10));
        parameters.add(radius.set("Radius", 1));
        parameters.add(targetTemperature.set("Target Temperature", 0.0f));
        parameters.add(coupling.set("Coupling", 0.0f));
        parameters.add(applyThermostat.set("Apply Thermostat", false));
        parameters.add(worldSize.set("World Size", glm::vec2(0.0f, 0.0f)));
        parameters.add(limitedFps.set("Limited FPS", true));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};

struct RenderParameters : public ParametersBase {
    ofParameter<int> size = 3;
    ofParameter<ofColor> color;
    ofParameter<glm::vec2> windowSize;
    ofParameter<bool> useShaders = false;
    ofParameter<bool> useFaketrails = false;
    ofParameter<bool> showVideoPreview = false;
    ofParameter<float> fakeTrialsVisibility = 0.0;
    ofParameter<float> videopreviewVisibility = 0.0;
    ofParameter<ofColor> videoColor;

    ofParameterGroup parameters;

    RenderParameters() {
        parameters.setName("Render Parameters");
        parameters.add(size.set("Size", 3));
        parameters.add(color.set("Color", ofColor::white));
        parameters.add(windowSize.set("Window Size", glm::vec2(0.0f, 0.0f)));
        parameters.add(useShaders.set("Use Shaders", false));
        parameters.add(useFaketrails.set("Use Fake Trails", false));
        parameters.add(showVideoPreview.set("Show Video Preview", false));
        parameters.add(fakeTrialsVisibility.set("Fake Trails Visibility", 0.0f));
        parameters.add(videopreviewVisibility.set("Video Preview Visibility", 0.0f));
        parameters.add(videoColor.set("Video Color", ofColor::black));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};


