#pragma once

#include "EsenciaPanelBase.h"

// Forward declaration
class Simulator;

class VACPanel : public EsenciaPanelBase {

    // Panel configuration
    const int PLOT_WIDTH = 300;
    const int PLOT_HEIGHT = 150;
    const ofRectangle PANEL_RECT = ofRectangle(12, 1, 10, 0);
    const ofColor BG_COLOR = ofColor(80, 80, 120, 100);

    // VAC plot parameters
    ofParameter<bool> enableVACCalculation;
    ofParameter<int> maxTimeLags;

    // Reference to simulator for accessing VAC data
    Simulator* simulator = nullptr;

public:
    void setup(ofxGui& gui, SimulationParameters& params, Simulator* sim);
    void onVACToggleChanged(bool& value);
    void onMaxTimeLagsChanged(int& value);
    void drawVACPlot(ofEventArgs& args);
};