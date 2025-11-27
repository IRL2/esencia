#pragma once

#include "EsenciaPanelBase.h"
#include <algorithm>

// Forward declaration
class Simulator;

class SonificationPanel : public EsenciaPanelBase {

    // Panel configuration
    const ofRectangle PANEL_RECT = ofRectangle(39, 2, 8, 0);
    const ofColor BG_COLOR = ofColor(80, 80, 120, 100);


    // Collision logging parameter
    ofParameter<bool> enableCollisionLogging;

    // Cluster analysis parameters
    ofParameter<bool> enableClusterAnalysis;
    ofParameter<float> clusterConnectionDistance;  // todo: declare this in the sonification parameters so it can be stored in presets

    ofxGuiGroup* vacGroup;

    // Reference to simulator for accessing VAC data
    Simulator* simulator = nullptr;
    SimulationParameters* simParams = nullptr;
    SonificationParameters* sonParams = nullptr;

public:

    float panelWidth = 250.0f; //initial values
    float panelHeight = 100.0f; 

    void setup(ofxGui& gui, SimulationParameters& simParams, SonificationParameters& sonParams, Simulator* sim);
    void onVACToggleChanged(bool& value);
    void onMaxTimeLagsChanged(int& value);
    void drawVACPlot(ofEventArgs& args);

    void onCollisionLoggingChanged(bool& value);
    void onClusterAnalysisChanged(bool& value);
    void onClusterConnectionDistanceChanged(float& value);
};