#include "VACPanel.h"
#include "../../simulation/simulator.h"
#include <algorithm>

void VACPanel::setup(ofxGui& gui, SimulationParameters& params, Simulator* sim) {
    simulator = sim;
    
    panel = gui.addPanel("VAC Analysis");
    
    // Add VAC control parameters sync with simulator state
    bool initialVACState = simulator ? simulator->isVACEnabled() : true;
    int initialMaxLags = simulator ? static_cast<int>(simulator->vacData.maxTimeLags) : 120;
    
    panel->add(enableVACCalculation.set("Enable VAC", initialVACState));
    panel->add(maxTimeLags.set("Max Time Lags", initialMaxLags, 10, 512));
    
    // VAC plot area
    ofxGuiContainer* plotContainer = panel->addContainer("VAC Plot", 
        ofJson({ {"width", PLOT_WIDTH}, {"height", PLOT_HEIGHT} }));
    
    enableVACCalculation.addListener(this, &VACPanel::onVACToggleChanged);
    maxTimeLags.addListener(this, &VACPanel::onMaxTimeLagsChanged);
    
    // Subscribe to draw events to render the plot
    ofAddListener(ofEvents().draw, this, &VACPanel::drawVACPlot);
    
    configVisuals(PANEL_RECT, BG_COLOR);
}

void VACPanel::onVACToggleChanged(bool& value) {
    if (simulator) {
        simulator->setVACEnabled(value);
        ofLogNotice("VACPanel") << "VAC calculation " << (value ? "enabled" : "disabled");
    }
}

void VACPanel::onMaxTimeLagsChanged(int& value) {
    if (simulator) {
        simulator->vacData.maxTimeLags = std::min(static_cast<uint32_t>(value), simulator->getMaxVelocityFrames());
        simulator->vacData.vacValues.resize(simulator->vacData.maxTimeLags, 0.0f);
        simulator->vacData.timePoints.resize(simulator->vacData.maxTimeLags);
        for (uint32_t i = 0; i < simulator->vacData.maxTimeLags; i++) {
            simulator->vacData.timePoints[i] = static_cast<float>(i) * 0.01f;
        }
        ofLogNotice("VACPanel") << "Max time lags changed to " << simulator->vacData.maxTimeLags;
    }
}

void VACPanel::drawVACPlot(ofEventArgs& args) {
    if (!simulator || !panel || !enableVACCalculation) return;
    
    ofVec2f panelPos = panel->getPosition();
    float panelWidth = panel->getWidth();
    float panelHeight = panel->getHeight();
    
    ofRectangle plotArea(panelPos.x + 40, panelPos.y + 140, PLOT_WIDTH - 50, PLOT_HEIGHT/2);
    
    // Get VAC data from simulator
    const VACData& vacData = simulator->vacData;
    
    if (vacData.vacValues.empty() || vacData.currentFrame < 2) return;
    
    ofPushStyle();
    
    // background
    ofSetColor(40, 40, 40, 180);
    ofFill();
    ofDrawRectangle(plotArea);
    
    // border
    ofSetColor(200, 200, 200);
    ofNoFill();
    ofSetLineWidth(1);
    ofDrawRectangle(plotArea);
    
    // axes
    ofSetColor(150, 150, 150);
    ofDrawLine(plotArea.x, plotArea.getBottom() - 1, plotArea.getRight(), plotArea.getBottom() - 1);
    ofDrawLine(plotArea.x + 1, plotArea.y, plotArea.x + 1, plotArea.getBottom());
    
    // VAC curve
    if (vacData.vacValues.size() > 1) {
        ofSetColor(100, 200, 255);
        ofSetLineWidth(3);
        
        uint32_t maxPoints = std::min(static_cast<uint32_t>(vacData.vacValues.size()), 
                                     static_cast<uint32_t>(maxTimeLags.get()));
        
        // Find min/max VAC values for scaling
        float minVAC = *std::min_element(vacData.vacValues.begin(), 
                                       vacData.vacValues.begin() + maxPoints);
        float maxVAC = *std::max_element(vacData.vacValues.begin(), 
                                       vacData.vacValues.begin() + maxPoints);
        
        // Ensure we have some range for plotting
        if (std::abs(maxVAC - minVAC) < 0.1f) {
            minVAC = -0.5f;
            maxVAC = 1.0f;
        }
        
        ofPolyline vacCurve;
        
        for (uint32_t i = 0; i < maxPoints; i++) {
            float x = plotArea.x + (static_cast<float>(i) / (maxPoints - 1)) * plotArea.width;
            float normalizedVAC = (vacData.vacValues[i] - minVAC) / (maxVAC - minVAC);
            float y = plotArea.getBottom() - normalizedVAC * plotArea.height;
            
            vacCurve.addVertex(x, y);
        }
        
        vacCurve.draw();
    }
    
    // Draw labels
    ofSetColor(200, 200, 200);
    
    ofDrawBitmapString("Velocity Autocorrelation Fn", plotArea.x-15, plotArea.y - 10);
    
    // X-axis 
    ofDrawBitmapString("Z(t)   /   Time (frames)", plotArea.x + 5, plotArea.getBottom() + 30);
    
    // Y-axis
    //ofPushMatrix();
    //ofTranslate(plotArea.x - 15, plotArea.y + (plotArea.height / 2) + 30);
    //ofRotate(-90);
    //ofDrawBitmapString("Z(t)", 0, 40);
    //ofPopMatrix();
    
    ofSetColor(255); // to-do: change for panel style text color
    
    // X-axis ticks (time)
    for (int i = 0; i <= 4; i++) {
        float x = plotArea.x + (i * plotArea.width / 4.0f);
        ofDrawLine(x, plotArea.getBottom() - 3, x, plotArea.getBottom() + 3);
        
        int timeValue = (maxTimeLags.get() * i / 4);
        ofDrawBitmapString(ofToString(timeValue), x - 10, plotArea.getBottom() + 15);
    }
    
    // Y-axis ticks (VAC values)
    for (int i = 0; i <= 3; i++) {
        float y = plotArea.getBottom() - (i * plotArea.height / 3.0f);
        ofDrawLine(plotArea.x - 3, y, plotArea.x + 3, y);
        
        float vacValue = -0.5f + (1.5f * i / 3.0f); // Range from -0.5 to 1.0
        ofDrawBitmapString(ofToString(vacValue, 1), plotArea.x - 33, y + 4);
    }
    
    ofPopStyle();
}