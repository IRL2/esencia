#pragma once

#include "ofxPDSP.h"


class AudioOscillator : public pdsp::Patchable {

public:
    AudioOscillator() { patch(); }
    AudioOscillator(const AudioOscillator& other) { patch(); }

    ~AudioOscillator() {}

    void patch() {
    }


};
