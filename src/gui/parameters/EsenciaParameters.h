#pragma once

#include "ParametersBase.h"





class CameraParameters : public ParametersBase {
public:
    ofParameter<bool> enableClipping = true;
    ofParameter<int> clipFar; // Threshold low
    ofParameter<int> clipNear; // Threshold hi

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

    ofParameterGroup parameters;

    CameraParameters() {
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }

};








struct PresetsParameters : public ParametersBase {

    // declare parameters for 16 buttons to select the state
    ofParameter<bool> states[16];

    // declare parameters for action buttons: save, clear, copyTo
    ofParameter<bool> save;
    ofParameter<bool> clear;
    ofParameter<bool> copyTo;

    ofParameterGroup parameters;

    PresetsParameters() {
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
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};






struct SimulationParameters : public ParametersBase {
    ofParameter<float> amount;
    ofParameter<int> radius;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> lowFps;

    ofParameterGroup parameters;

    SimulationParameters() {
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};




