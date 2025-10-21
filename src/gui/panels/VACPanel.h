#pragma once

#include "EsenciaPanelBase.h"

// Forward declaration
class Simulator;

class VACPanel : public EsenciaPanelBase {

    // Panel configuration
    const ofRectangle PANEL_RECT = ofRectangle(37, 6, 8, 0);
    const ofColor BG_COLOR = ofColor(80, 80, 120, 100);

    // VAC plot parameters
    ofParameter<bool> enableVACCalculation;
    ofParameter<int> maxTimeLags;

    // Reference to simulator for accessing VAC data
    Simulator* simulator = nullptr;

public:
    const int PLOT_WIDTH = 250;
    const int PLOT_HEIGHT = 140;

    void setup(ofxGui& gui, SimulationParameters& params, Simulator* sim);
    void onVACToggleChanged(bool& value);
    void onMaxTimeLagsChanged(int& value);
    void drawVACPlot(ofEventArgs& args);
};