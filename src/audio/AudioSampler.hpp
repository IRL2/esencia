#pragma once

#include "ofxPDSP.h"


class AudioSampler : public pdsp::Patchable {

public:
    AudioSampler() { patch(); }
    AudioSampler(const AudioSampler& other) { patch(); }

    ~AudioSampler() { }

    void patch() {

        addModuleInput("trig", triggers);
        addModuleInput("position", sampler.in_start());
        addModuleInput("pitch", sampler.in_pitch());
        addModuleInput("select", sampler.in_select());
        addModuleOutput("signal", amp);

        env.enableDBTriggering(-24.0f, 0.0f);
        setAHR(0.0f, 0.0f, 2000.0f); // all the samples used here are shorter than this

        trig >> triggers;
        triggers >> env >> amp.in_mod();

        triggers >> sampler >> lowcut >> reverb >> verbGain >> amp;

        timeControl >> reverb.in_time();
        densityControl >> reverb.in_density();
        dampingControl >> reverb.in_damping();
        hiCutControl >> reverb.in_hi_cut();
        modFreqControl >> reverb.in_mod_freq();
        modAmountControl >> reverb.in_mod_amount();

        lowCutControl.set("low cut freq", 100, 20, 1000);
        timeControl.set("rt60", 3.33f, 0.05f, 20.0f);
        densityControl.set("density", 0.5f, 0.0f, 1.0f);
        dampingControl.set("damping", 0.5f, 0.0f, 1.0f);
        hiCutControl.set("high cut freq", 5000, 3000, 20000);
        modFreqControl.set("mod speed (hz)", 0.2f, 0.01f, 1.25f);
        modAmountControl.set("mod depth (ms)", 0.8f, 0.0f, 3.0f);
    }

    void load(string path, bool setHoldTime = false) {
        //sample = new pdsp::SampleBuffer();
        sample.load(path);
        if (sample.channels == 1) {
            sampler.setSample(&sample, 0, 0);
        }
        else {
            sampler.setSample(&sample, 0, 1);
        }
    }

    void gain(float dBvalue) {
        sampler* dB(dBvalue) >> amp;
    }

    float meter_env() const {
        return env.meter_output();
    }

    float meter_position() const {
        return sampler.meter_position();
    }

    void setAHR(float attack, float hold, float release) {
        env.set(attack, hold, release);
    }

    void setReverb(float density, float time, float damping, float hiCut, float modFreq, float modAmount, float verbLevel) {
        densityControl.set(density);
        timeControl.set(time);
        dampingControl.set(damping);
        hiCutControl.set(hiCut);
        modFreqControl.set(modFreq);
        modAmountControl.set(modAmount);
        verbGain.set(verbLevel);
    }

    void play(float pitch, float volume) {
        //if (sampler.meter_position() > 0.0f && sampler.meter_position() < 0.99f) {
            //0.0 >> sampler.in_start(); // reset position if we are at the end of the sample
            //return;
        //}

        ofLog() << "trigger audio sampler with pitch " << pitch  << " and volume " << volume;
        pitch >> sampler.in_pitch();
        volume >> amp.in_mod();
        trig.trigger(0.5f); // default volume envelop
    }


    pdsp::TriggerControl trig;

    ofParameterGroup parameters;
    pdsp::Parameter     timeControl;
    pdsp::Parameter     densityControl;
    pdsp::Parameter     dampingControl;
    pdsp::Parameter     hiCutControl;
    pdsp::Parameter     modFreqControl;
    pdsp::Parameter     modAmountControl;
    pdsp::Parameter     lowCutControl;
    pdsp::ParameterGain	verbGain;


    pdsp::BasiVerb reverb;
    pdsp::PatchNode     triggers;
    pdsp::Sampler       sampler;
    pdsp::AHR           env;
    pdsp::Amp           amp;
    pdsp::LowCut        lowcut;

    pdsp::SampleBuffer sample;

};
