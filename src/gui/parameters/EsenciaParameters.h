#pragma once

#include "ParametersBase.h"




/// <summary>
/// The CameraParameters struct contains all the parameters that are used to configure the camera.
/// </summary>
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


    CameraParameters() {
        initializeParameterMap();
    }

    void initializeParameterMap() {
        parameterMap["camera"] = nullptr;

        parameterMap["enableClipping"] = &enableClipping;
        parameterMap["clipFar"] = &clipFar;
        parameterMap["clipNear"] = &clipNear;
        parameterMap["blobMinArea"] = &blobMinArea;
        parameterMap["blobMaxArea"] = &blobMaxArea;
        parameterMap["gaussianBlur"] = &gaussianBlur;
        parameterMap["nConsidered"] = &nConsidered;
        parameterMap["fillHolesOnPolygons"] = &fillHolesOnPolygons;
        parameterMap["floodfillHoles"] = &floodfillHoles;
        parameterMap["polygonTolerance"] = &polygonTolerance;
        parameterMap["showPolygons"] = &showPolygons;
        parameterMap["startBackgroundReference"] = &startBackgroundReference;
        parameterMap["saveDebugImages"] = &saveDebugImages;
        parameterMap["recordTestingVideo"] = &recordTestingVideo;
        parameterMap["useMask"] = &useMask;
        parameterMap["_sourceOrbbec"] = &_sourceOrbbec;
        parameterMap["_sourceVideofile"] = &_sourceVideofile;
        parameterMap["_sourceWebcam"] = &_sourceWebcam;
    }

};







/// <summary>
/// The PresetsParameters struct contains all the parameters that are used to configure the presets.
/// </summary>
struct PresetsParameters : public ParametersBase {

    // the following ites are only gui holders
    // 
    // declare parameters for 16 buttons to select the state
    ofParameter<bool> states[16];

    // declare parameters for action buttons: save, clear, copyTo
    ofParameter<bool> save;
    ofParameter<bool> clear;
    ofParameter<bool> copyTo;

    ofParameterGroup parameters; // a copy of all the parameters for acting the presets.. maybe

    // the following are the actual parameter info
    ofParameter<int> activePreset = { 0 };
    ofParameter<int> nextPreset = { 0 };
    ofParameter<float> transitionDuration = { 0.0 };
    ofParameter<bool> isTransitioning = { false };


    PresetsParameters() {
        initializeParameterMap();
    }

    void initializeParameterMap() {
		parameterMap["presets"] = nullptr;

        for (int i = 0; i < 16; i++) {
            parameterMap["states[" + ofToString(i) + "]"] = &states[i];
        }
        parameterMap["save"] = &save;
        parameterMap["clear"] = &clear;
        parameterMap["copyTo"] = &copyTo;
        parameterMap["activePreset"] = &activePreset;
        parameterMap["nextPreset"] = &nextPreset;
        parameterMap["transitionDuration"] = &transitionDuration;
        parameterMap["isTransitioning"] = &isTransitioning;
    }
};





/// <summary>
/// The RenderParameters struct contains all the parameters that are used to configure the rendering.
/// </summary>
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

    RenderParameters() {
		initializeParameterMap();
    }

	void initializeParameterMap() {
        parameterMap["render"] = nullptr;

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
struct SimulationParameters : public ParametersBase {
    ofParameter<float> amount;
    ofParameter<int> radius;
    ofParameter<float> targetTemperature;
    ofParameter<float> coupling;
    ofParameter<bool> applyThermostat;
    ofParameter<glm::vec2> worldSize;
    ofParameter<bool> lowFps;

    SimulationParameters() {
		initializeParameterMap();
    }

	void initializeParameterMap() {
        parameterMap["simulation"] = nullptr;

		parameterMap["amount"] = &amount;
		parameterMap["radius"] = &radius;
		parameterMap["targetTemperature"] = &targetTemperature;
		parameterMap["coupling"] = &coupling;
		parameterMap["applyThermostat"] = &applyThermostat;
		parameterMap["worldSize"] = &worldSize;
		parameterMap["lowFps"] = &lowFps;
	}
};




