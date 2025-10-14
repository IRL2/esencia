#include "SimulationDataPanel.h"
#include "../../simulation/simulator.h"

void SimulationDataPanel::setup(ofxGui& gui, SimulationParameters& params, Simulator* sim) {
    simulator = sim;
    
    panel = gui.addPanel("Simulation Data");
    
    // Add collision logging control parameter
    bool initialCollisionLoggingState = simulator ? simulator->isCollisionLoggingEnabled() : false;
    panel->add(enableCollisionLogging.set("Enable Collision Logging", initialCollisionLoggingState));
    
    // Add cluster analysis control parameters
    bool initialClusterAnalysisState = simulator ? simulator->isClusterAnalysisEnabled() : true;
    float initialClusterDistance = simulator ? simulator->getClusterConnectionDistance() : 50.0f;
    
    panel->add(enableClusterAnalysis.set("Enable Cluster Analysis", initialClusterAnalysisState));
    panel->add(clusterConnectionDistance.set("Cluster Connection Distance", initialClusterDistance, 10.0f, 200.0f));
    
    // Add listeners
    enableCollisionLogging.addListener(this, &SimulationDataPanel::onCollisionLoggingChanged);
    enableClusterAnalysis.addListener(this, &SimulationDataPanel::onClusterAnalysisChanged);
    clusterConnectionDistance.addListener(this, &SimulationDataPanel::onClusterConnectionDistanceChanged);
    
    configVisuals(PANEL_RECT, BG_COLOR);
}

void SimulationDataPanel::onCollisionLoggingChanged(bool& value) {
    if (simulator) {
        simulator->setCollisionLoggingEnabled(value);
        ofLogNotice("SimulationDataPanel") << "Collision logging " << (value ? "enabled" : "disabled") << " via GUI panel";
    }
}

void SimulationDataPanel::onClusterAnalysisChanged(bool& value) {
    if (simulator) {
        simulator->setClusterAnalysisEnabled(value);
        ofLogNotice("SimulationDataPanel") << "Cluster analysis " << (value ? "enabled" : "disabled") << " via GUI panel";
    }
}

void SimulationDataPanel::onClusterConnectionDistanceChanged(float& value) {
    if (simulator) {
        simulator->setClusterConnectionDistance(value);
        ofLogNotice("SimulationDataPanel") << "Cluster connection distance changed to " << value << " via GUI panel";
    }
}