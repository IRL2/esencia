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
    

    void logCollisionDetails(const CollisionBuffer& collisionData);
    void processCollisionsForAudio(const CollisionBuffer& collisionData);

    // New cluster analysis methods
    void logClusterDetails(const ClusterAnalysisData& clusterData);
    void processClusterStatistics(const ClusterAnalysisData& clusterData);

    CollisionBuffer* collisionData = nullptr;
    ClusterAnalysisData* clusterData = nullptr;

private:

    SonificationParameters* parameters = nullptr;
    GuiApp* allParameters = nullptr;

    const bool DEBUG_LOG = false;

    bool noteSent = false;


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
    DataSynth       dataSynth;

    pdsp::ParameterAmp  masterAmp;

    pdsp::Scope mainScope;

    int lastTime;
};
