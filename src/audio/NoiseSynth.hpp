#pragma once

#include "ofxPDSP.h"


class NoiseSynth : public pdsp::Patchable {

public:
    pdsp::WhiteNoise noise;
    pdsp::ParameterAmp reverbDryGain;
    pdsp::ParameterAmp amp;
    pdsp::DimensionChorus chorus;

    pdsp::VAFilter filter;
    pdsp::BasiVerb reverb;

    ofParameterGroup    ui;
    pdsp::Parameter  filterModeControl;
    pdsp::Parameter  filterFreqControl;
    pdsp::Parameter  filterResoControl;
    pdsp::Parameter  filterCutoffControl;
    pdsp::Parameter  reverbFreqControl;
    pdsp::Parameter  reverbModControl;
    pdsp::Parameter  reverbTimeControl;
    pdsp::Parameter  reverbDampingControl;
    pdsp::Parameter  reverbDensityControl;
    pdsp::Parameter  chorusDelayControl;
    pdsp::Parameter  chorusDepthControl;
    pdsp::Parameter  chorusSpeedControl;

    bool isPlaying = false;

    NoiseSynth() { patch(); }
    NoiseSynth(const NoiseSynth& other) { patch(); }

    ~NoiseSynth() {}

    void patch() {
        addModuleOutput("signal", amp);

        noise >> filter >> chorus >> reverb >> amp;
        filter >> reverbDryGain >> amp;

        filterModeControl >> filter.in_mode();
        filterFreqControl >> filter.in_pitch();
        filterResoControl >> filter.in_reso();
        filterCutoffControl >> filter.in_cutoff();

        reverbFreqControl >> reverb.in_mod_freq();
        reverbTimeControl >> reverb.in_time();
        reverbDampingControl >> reverb.in_damping();
        reverbDensityControl >> reverb.in_density();
        reverbModControl >> reverb.in_mod_amount();

        chorusDelayControl >> chorus.in_delay();
        chorusDepthControl >> chorus.in_depth();
        chorusSpeedControl >> chorus.in_speed();

        amp.set(0);
        amp.enableSmoothing(1000.0f);

        ui.setName("noiseSynth");
        ui.add(amp.set("gain", 0.6f, 0.0f, 1.0f));
        ui.add(reverbTimeControl.set("reverb time", 1.5f, 0.0f, 20.0f));
        ui.add(reverbDampingControl.set("reverb damping", 0.5f, 0.0f, 1.0f));
        ui.add(reverbDensityControl.set("reverb density", 0.5f, 0.0f, 1.0f));
        ui.add(reverbFreqControl.set("reverb mod freq Hz", 0.2f, 0.0f, 2.0f));
        ui.add(reverbModControl.set("reverb mod amount", 0.2f, 0.0f, 2.0f));
        ui.add(reverbDryGain.set("reverb dry", 0.3f, 0.0f, 1.0f));

        ui.add(filterModeControl.set("filter mode", 0, 0, 6));
        ui.add(filterFreqControl.set("filter freq", 120.0f, 0.0f, 120.0f));
        ui.add(filterResoControl.set("filter reso", 0.0f, 0.0f, 1.1f));

        ui.add(chorusDelayControl.set("chorus delay ms", 50.0f, 0.0f, 100.0f));
        ui.add(chorusDepthControl.set("chorus depth ms", 20.0f, 0.0f, 30.0f));
        ui.add(chorusSpeedControl.set("chorus speed Hz", 2.25f, 0.0f, 5.0f));
    }

    void play(float velocity = 1.0f) {
        if (isPlaying) return;
        amp.set(velocity);
        isPlaying = true;
    }

    void stop() {
        if (!isPlaying) return;
        amp.set(0);
        isPlaying = false;
    }

    void setFilter(float mode, float freq, float res) {
        filterModeControl.set(mode);
        filterFreqControl.set(freq);
        filterResoControl.set(res);
    }

    void setReverb(float time, float damping, float density, float modfreq, float dry) {
        reverbTimeControl.set(time);
        reverbFreqControl.set(modfreq);
        reverbDampingControl.set(damping);
        reverbDensityControl.set(density);
        reverbDryGain.set(dry);
    }

};
