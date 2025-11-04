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
#include "NoiseSynth.hpp"

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
    //bool triggerAtGate(const std::vector<float>& singleBeatPattern, float beatDurationSeconds, std::function<void()> callback);
    //bool triggerAtGate(const std::vector<std::vector<float>>& patternBeats, float beatDurationSeconds, std::function<void()> callback);
    bool checkInterval(float intervalInSeconds);

    // scenes
    void setupCollisionSounds(int bank=0);
    void setupVelocityNoise();
    void setupClusterSounds(int bank = 0);
    void playCollisionSounds(float speedFactor=1.0);
    void playVelocityNoise();
    void playClusterSounds();
    void playEmpty();
    void stopClusterSounds();
    void stopCollisionSounds();
    void stopVelocityNoise();
    void stopEmpty();
    void stopAll();

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

    // not in use
    AudioSampler     sampler1;
    AudioSampler     sampler2;
    AudioOscillator  oscillator1;
    AudioOscillator  oscillator2;
    PolySynth        polySynth;
    DataSynth        dataSynth;

    AudioSampler    collisionSampler1;
    AudioSampler    collisionSampler2;
    AudioSampler    clusterSampler1;
    AudioSampler    clusterSampler2;
    AudioSampler    clusterSampler3;
    PolySynth       clusterSynth1;
    DataSynth       clusterDataSynth1;
    NoiseSynth      noiseSynth;
    AudioSampler    ambienceSampler;

    // mixer
    pdsp::ParameterAmp  master;
    pdsp::ParameterAmp  collisionTrack;
    pdsp::ParameterAmp  clusterTrack;
    pdsp::ParameterAmp  velocityTrack;
    pdsp::ParameterAmp  backgroundTrack;

    // final effects
    pdsp::Compressor masterCompressor;
    pdsp::BasiVerb   masterReverb;
    pdsp::Panner     panner;

    // final eq
    pdsp::PeakEQ      midEQ;
    pdsp::LowShelfEQ  lowEQ;
    pdsp::HighShelfEQ highEQ;

    // scopes
    pdsp::Scope mainScope;       
    pdsp::Scope collisionScope;  
    pdsp::Scope clustersScope;   
    pdsp::Scope backgroundScope; 
    pdsp::Scope velocityScope;   
    int scopeHeight;

    int lastTime;

    void windowResize(ofResizeEventArgs&);
};
