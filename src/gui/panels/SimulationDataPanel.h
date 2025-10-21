#pragma once

#include "EsenciaPanelBase.h"

// Forward declaration
class Simulator;

class SimulationDataPanel : public EsenciaPanelBase {

    const ofRectangle PANEL_RECT = ofRectangle(34, 1, 10, 0);
    const ofColor BG_COLOR = ofColor(200, 120, 80, 100);

    // Collision logging parameter
    ofParameter<bool> enableCollisionLogging;
    
    // Cluster analysis parameters
    ofParameter<bool> enableClusterAnalysis;
    ofParameter<float> clusterConnectionDistance;

    Simulator* simulator = nullptr;

public:
    void setup(ofxGui& gui, SimulationParameters& params, Simulator* sim);
    void onCollisionLoggingChanged(bool& value);
    void onClusterAnalysisChanged(bool& value);
    void onClusterConnectionDistanceChanged(float& value);
};