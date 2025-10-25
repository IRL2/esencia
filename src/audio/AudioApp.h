#pragma once

#include "ofMain.h"
#include <vector>
#include <iomanip>

// sound system
#include "ofxPDSP.h"
#include "AudioSampler.hpp"
#include "AudioOscillator.hpp"
#include "PolySynth.hpp"
#include "DataSynth.hpp"

#include "EsenciaParameters.h"
#include "GuiApp.h"

// Forward declarations
struct CollisionData;
struct CollisionBuffer;
struct ClusterStats;
struct ClusterAnalysisData;

class AudioApp {
public:
    void setup(SonificationParameters* params, GuiApp* gui);
    void update();
    void draw();

    // collision processing
    void logCollisionDetails(const CollisionBuffer& collisionData);
    void processCollisionsStatistics(const CollisionBuffer& collisionData);

    // cluster analysis
    void logClusterDetails(const ClusterAnalysisData& clusterData);
    void processClusterStatistics(const ClusterAnalysisData& clusterData);

    // when no cluster/collision has detected, needs to reports zeros
    void cleanClusterStatistics();
    void cleanCollisionStatistics();

    // audio processing
    void sonificationControl(const CollisionBuffer& collisionData, const ClusterAnalysisData& clusterData);

    CollisionBuffer* collisionData = nullptr;
    ClusterAnalysisData* clusterData = nullptr;

    // timming functions for audio triggers
    bool triggerAtInterval(float intervalInSeconds, std::function<void()> callback);
    bool checkInterval(float intervalInSeconds);

private:

    SonificationParameters* parameters = nullptr;
    GuiApp* allParameters = nullptr;

    bool DEBUG_LOG = false;

    uint32_t lastProcessedFrame = 0;
    uint32_t lastProcessedClusterFrame = 0;
    bool audioEnabled = true;
    bool clusterAnalysisEnabled = true;


    // sound modules and instruments
    pdsp::Engine   audioEngine;

    AudioSampler     sampler1;
    AudioSampler     sampler2;
    AudioOscillator  oscillator1;
    AudioOscillator  oscillator2;
    PolySynth        polySynth;
    DataSynth        dataSynth;

    pdsp::ParameterAmp  masterAmp;

    pdsp::Scope mainScope;

    int lastTime;
};
