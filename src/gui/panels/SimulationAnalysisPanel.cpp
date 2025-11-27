#include "SimulationAnalysisPanel.h"
#include "simulator.h"

void SonificationPanel::setup(ofxGui& gui, SimulationParameters& simParams, SonificationParameters& sonParams, Simulator* sim) {
    simulator = sim;
    this->simParams = &simParams;
    this->sonParams = &sonParams;

    panel = gui.addPanel("sonification analysis");

    // Add collision logging control parameter
    bool initialCollisionLoggingState = simulator ? simulator->isCollisionLoggingEnabled() : false;
    enableCollisionLogging.set("Enable Collision Logging", initialCollisionLoggingState);

    // Add cluster analysis control parameters
    bool initialClusterAnalysisState = simulator ? simulator->isClusterAnalysisEnabled() : true;
    float initialClusterDistance = simulator ? simulator->getClusterConnectionDistance() : 50.0f;

    enableClusterAnalysis.set("Enable Cluster Analysis", initialClusterAnalysisState);
    panel->add(clusterConnectionDistance.set("cluster connection distance", initialClusterDistance, 10.0f, 200.0f), ofJson({ {"precision", 0} }));


    // Add listeners
    enableCollisionLogging.addListener(this, &SonificationPanel::onCollisionLoggingChanged);
    enableClusterAnalysis.addListener(this, &SonificationPanel::onClusterAnalysisChanged);
    clusterConnectionDistance.addListener(this, &SonificationPanel::onClusterConnectionDistanceChanged);


    vacGroup = panel->addGroup("velocity auto-correlation");
  
    // Add VAC control parameters sync with simulator state
    bool initialVACState = simulator ? simulator->isVACEnabled() : true;
    int initialMaxLags = simulator ? static_cast<int>(simulator->vacData.maxTimeLags) : 120;
    
    vacGroup->add(this->sonParams->enableVACCalculation.set("enable VAC", initialVACState));
    vacGroup->add(this->sonParams->maxTimeLags.set("max time lags", initialMaxLags, 10, 512));
    
    // VAC plot area
    ofxGuiContainer* plotContainer = vacGroup->addContainer("VAC Plot",
        ofJson({ {"width", panelWidth-10}, {"height", panelHeight*2} }));
    
    this->sonParams->enableVACCalculation.addListener(this, &SonificationPanel::onVACToggleChanged);
    this->sonParams->maxTimeLags.addListener(this, &SonificationPanel::onMaxTimeLagsChanged);
    
    // Subscribe to draw events to render the plot
    ofAddListener(ofEvents().draw, this, &SonificationPanel::drawVACPlot);
    
    configVisuals(PANEL_RECT, BG_COLOR);
}


void SonificationPanel::onCollisionLoggingChanged(bool& value) {
    if (simulator) {
        simulator->setCollisionLoggingEnabled(value);
        //ofLogNotice("SimulationDataPanel") << "Collision logging " << (value ? "enabled" : "disabled") << " via GUI panel";
    }
}

void SonificationPanel::onClusterAnalysisChanged(bool& value) {
    if (simulator) {
        simulator->setClusterAnalysisEnabled(value);
        //ofLogNotice("SimulationDataPanel") << "Cluster analysis " << (value ? "enabled" : "disabled") << " via GUI panel";
    }
}

void SonificationPanel::onClusterConnectionDistanceChanged(float& value) {
    if (simulator) {
        simulator->setClusterConnectionDistance(value);
        //ofLogNotice("SimulationDataPanel") << "Cluster connection distance changed to " << value << " via GUI panel";
    }
}


void SonificationPanel::onVACToggleChanged(bool& value) {
    if (simulator) {
        simulator->setVACEnabled(value);
        ofLogNotice("SonificationPanel") << "VAC calculation " << (value ? "enabled" : "disabled");
    }
}

void SonificationPanel::onMaxTimeLagsChanged(int& value) {
    if (simulator) {
        simulator->vacData.maxTimeLags = std::min(static_cast<uint32_t>(value), simulator->getMaxVelocityFrames());
        simulator->vacData.vacValues.resize(simulator->vacData.maxTimeLags, 0.0f);
        simulator->vacData.timePoints.resize(simulator->vacData.maxTimeLags);
        for (uint32_t i = 0; i < simulator->vacData.maxTimeLags; i++) {
            simulator->vacData.timePoints[i] = static_cast<float>(i) * 0.01f;
        }
        //ofLogNotice("SonificationPanel") << "Max time lags changed to " << simulator->vacData.maxTimeLags;
    }
}

void SonificationPanel::drawVACPlot(ofEventArgs& args) {
    if (!simulator || !panel || !this->sonParams->enableVACCalculation.get()) return;
    if (vacGroup->getVisible() == false) return;
    if (vacGroup->getMinimized()) return;

    ofVec2f panelPos = vacGroup->getPosition() + panel->getPosition();
    panelWidth = vacGroup->getWidth();
    panelHeight = vacGroup->getHeight();
    
    ofRectangle plotArea(panelPos.x + 40, panelPos.y + 140, panelWidth - 50, panelHeight/3);
    
    // Get VAC data from simulator
    const VACData& vacData = simulator->vacData;
    
    if (vacData.vacValues.empty() || vacData.currentFrame < 2) return;
    sonParams->vacValues = vacData.vacValues;
    sonParams->vacWidth = vacData.vacValues.size();
    sonParams->vacHeight = vacData.vacValues.size() > 0 ? static_cast<int>(*std::max_element(vacData.vacValues.begin(), vacData.vacValues.end())) : 0;
    
    
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
                                     static_cast<uint32_t>(this->sonParams->maxTimeLags.get()));
        
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
    
    ofSetColor(255); // to-do: change for panel style text color
    
    // X-axis ticks (time)
    for (int i = 0; i <= 4; i++) {
        float x = plotArea.x + (i * plotArea.width / 4.0f);
        ofDrawLine(x, plotArea.getBottom() - 3, x, plotArea.getBottom() + 3);
        
        int timeValue = (this->sonParams->maxTimeLags.get() * i / 4);
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