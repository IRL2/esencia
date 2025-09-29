#pragma once

#include "ofxPDSP.h"
#include "AudioSampler.hpp"
#include "AudioOscillator.hpp"

class SoundManager {

public:
    void setup();
    void guiDraw();

    void setVolume(float volume);

    

private:
    pdsp::Engine   engine;

    pdsp::Function  rseq;
    pdsp::Function  dseq;

    AudioSampler     sampler1;
    AudioOscillator  oscillator1;

    pdsp::Scope mainScope;
    //pdsp::Scope sample1Scope;
    //pdsp::Scope oscillator1Scope;
};