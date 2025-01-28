#pragma once

#include "ofxPresetsParametersBase.h"

#include <ofImage.h>



/// <summary>
/// The CameraParameters struct contains all the parameters that are used to configure the camera.
/// </summary>
struct CameraParameters : public ofxPresetsParametersBase {
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

    // TODO: these parameters are used for create buttons on the gui
    // should be moved to the corresponding panel
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


    CameraParameters() {
        groupName = "camera";

        parameterMap["enableClipping"] = &enableClipping;
        //parameterMap["clipFar"] = &clipFar;
        //parameterMap["clipNear"] = &clipNear;
        parameterMap["blobMinArea"] = &blobMinArea;
        parameterMap["blobMaxArea"] = &blobMaxArea;
        parameterMap["gaussianBlur"] = &gaussianBlur;
        parameterMap["nConsidered"] = &nConsidered;
        parameterMap["fillHolesOnPolygons"] = &fillHolesOnPolygons;
        parameterMap["floodfillHoles"] = &floodfillHoles;
        parameterMap["polygonTolerance"] = &polygonTolerance;
        parameterMap["showPolygons"] = &showPolygons;
        parameterMap["useMask"] = &useMask;
    }

};







/// <summary>
/// The PresetsParameters struct contains all the parameters that are used to configure the presets.
/// </summary>
struct PresetsParameters : public ofxPresetsParametersBase {

    // the following ites are only gui holders
    // 
    // declare parameters for 16 buttons to select the state
    ofParameter<bool> states[16];

    // declare parameters for action buttons: save, clear, copyTo

    ofParameterGroup parameters; // a copy of all the parameters for acting the presets.. maybe

    // the following are the actual parameter info
    ofParameter<int> activePreset = { 0 };
    ofParameter<int> nextPreset = { 0 };

    ofParameter<float> transitionDuration = { 0.0 };
    ofParameter<float> presetDuration = { 0.0 };
    ofParameter<std::string> sequence;
	ofParameter<int> sequenceIndex = { 0 };


    PresetsParameters() {
		groupName = "presets";

        parameterMap["sequence"] = &sequence;
        //parameterMap["transitionDuration"] = &transitionDuration;
        //parameterMap["presetDuration"] = &transitionDuration;

        // TODO: add presetParameters to the parameterMap (on the presetsPanel setup, that is the one who initializes the PresetManager)
        // TODO: should we move the PresetManager setup to the GuiApp?
    }
};







/// <summary>
/// The RenderParameters struct contains all the parameters that are used to configure the rendering.
/// </summary>
struct RenderParameters : public ofxPresetsParametersBase {
    ofParameter<int> size = 3;
    ofParameter<ofColor> color;
    ofParameter<glm::vec2> windowSize;
    ofParameter<bool> useShaders = true;
    ofParameter<bool> useFaketrails = true;
    ofParameter<bool> showVideoPreview = true;
    ofParameter<float> fakeTrialsVisibility = 0.0;
    ofParameter<float> videopreviewVisibility = 0.0;
    ofParameter<ofColor> videoColor;

    RenderParameters() {
        groupName = "render";

		parameterMap["size"] = &size;
		parameterMap["color"] = &color;
		parameterMap["windowSize"] = &windowSize;
		parameterMap["useShaders"] = &useShaders;
		parameterMap["useFaketrails"] = &useFaketrails;
		parameterMap["showVideoPreview"] = &showVideoPreview;
		parameterMap["fakeTrialsVisibility"] = &fakeTrialsVisibility;
		parameterMap["videopreviewVisibility"] = &videopreviewVisibility;
		parameterMap["videoColor"] = &videoColor;
	}
};





/// <summary>
/// The SimulationParameters struct contains all the parameters that are used to configure the simulation.
/// </summary>
struct SimulationParameters : public ofxPresetsParametersBase {
    ofParameter<float> amount;
    ofParameter<int> radius;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<float> depthFieldScale;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> lowFps;

    SimulationParameters() {
		groupName = "simulation";

		parameterMap["amount"] = &amount;
		parameterMap["radius"] = &radius;
		parameterMap["targetTemperature"] = &targetTemperature;
		parameterMap["coupling"] = &coupling;
		parameterMap["applyThermostat"] = &applyThermostat;
		parameterMap["depthFieldScale"] = &depthFieldScale;
		//parameterMap["worldSize"] = &worldSize;
		//parameterMap["lowFps"] = &lowFps;
	}
};




