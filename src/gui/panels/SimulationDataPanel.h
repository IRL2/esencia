#pragma once

#include "EsenciaPanelBase.h"

// Forward declaration
class Simulator;

class SimulationDataPanel : public EsenciaPanelBase {

    const ofRectangle PANEL_RECT = ofRectangle(39, 1, 8, 0);
    const ofColor BG_COLOR = ofColor(200, 120, 80, 100);

    // Collision logging parameter
    ofParameter<bool> enableCollisionLogging;
    
    // Cluster analysis parameters
    ofParameter<bool> enableClusterAnalysis;

    ofParameter<float> clusterConnectionDistance;  // todo: declare this in the sonification parameters so it can be stored in presets

    Simulator* simulator = nullptr;

public:
    void setup(ofxGui& gui, SimulationParameters& params, Simulator* sim);
    void onCollisionLoggingChanged(bool& value);
    void onClusterAnalysisChanged(bool& value);
    void onClusterConnectionDistanceChanged(float& value);
};