#pragma once

#include "ofxPDSP.h"
#include "AudioSampler.hpp"
#include "AudioOscillator.hpp"
#include "PolySynth.hpp"
#include "DataSynth.hpp"

class SoundManager {

public:
    void setup();
    void guiDraw();

    void setVolume(float volume);

    

private:
    pdsp::Engine   audioEngine;

    PolySynth      polysynth;
    DataSynth      datasynth;

    AudioSampler     audiosampler;
    AudioOscillator  oscillator;

    pdsp::Scope mainScope;
};