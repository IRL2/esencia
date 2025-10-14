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

        sampler >> delay >> delayGain >> amp;
        sampler >> amp;

        damp >> amp.in_mod();

        amp >> fader; // output

        timeControl >> reverb.in_time();
        densityControl >> reverb.in_density();
        dampingControl >> reverb.in_damping();
        hiCutControl >> reverb.in_hi_cut();
        modFreqControl >> reverb.in_mod_freq();
        modAmountControl >> reverb.in_mod_amount();

        pitchControl >> sampler.in_pitch();

        delayTimeControl * 600.0f >> delay.in_time();
        delayFeedbackControl >> delay.in_feedback();
        delayDampControl >> delay.in_damping();

        ui.setName("audio sampler");
        // reverb
        ui.add(verbGain.set("reverb gain", 9, -48, 12));
        ui.add(lowCutControl.set("low cut freq", 100, 20, 1000));
        ui.add(hiCutControl.set("high cut freq", 5000, 3000, 20000));
        ui.add(timeControl.set("rt60", 3.33f, 0.05f, 20.0f));
        ui.add(densityControl.set("density", 0.5f, 0.0f, 1.0f));
        ui.add(dampingControl.set("damping", 0.5f, 0.0f, 1.0f));
        ui.add(modFreqControl.set("mod speed (hz)", 0.2f, 0.01f, 1.25f));
        ui.add(modAmountControl.set("mod depth (ms)", 0.8f, 0.0f, 3.0f));
        // delay
        ui.add(delayGain.set("delay gain", -3, -48, 12));
        ui.add(delayTimeControl.set("delay time", 0.3f, 0.0f, 10.0f));
        ui.add(delayFeedbackControl.set("delay feedback", 0.3f, 0.0f, 0.95f));
        ui.add(delayDampControl.set("delay damp", 0.5f, 0.0f, 1.0f));

        damp.set("damp", 0.0);

        fader.set("gain", -12, -48, 12);

        pitchControl.enableSmoothing(50.0f);
    }

    void add(string path, bool setHoldTime = false) {
        samples.push_back(new pdsp::SampleBuffer());
        samples.back()->load(path);
        sampler.addSample(samples.back());
    }

    size_t getNumSamples() const {
        return samples.size();
    }

    void gain(float dBvalue) {
        sampler* dB(dBvalue) >> amp.in_mod();
        ofLog() << "gain changed to " << dBvalue;
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

    void setReverb(float gain, float density, float time, float damping, float hiCut, float modFreq, float modAmount = NULL) {
        verbGain.set(gain);
        densityControl.set(density);
        timeControl.set(time);
        dampingControl.set(damping);
        hiCutControl.set(hiCut);
        modFreqControl.set(modFreq);
        modAmount ? modAmountControl.set(modAmount) : true;
    }

    void setDelay(float time, float feedback) {
        delayTimeControl.set(time);
        delayFeedbackControl.set(feedback);
    }

    void play(float pitch, int sampleIndex = 0, bool restart = true, float velocity = 1.0, float damp = 1.0) {
        if (restart) {
            0.0 >> sampler.in_start(); // reset position if we are at the end of the sample
        }
        else {
            if (sampler.meter_position() > 0.0f && sampler.meter_position() < 0.99f) {
                return;
            }
        }

        currentSampleIndex = sampleIndex;
        currentSampleIndex >> sampler.in_select();

        pitch >> sampler.in_pitch();
        damp >> amp.in_mod();
        this->damp.set (damp);

        trig.trigger(velocity);
    }

    void switchSampleIndex(int sampleIndex) {
        ofLog() << "switching to sample index " << sampleIndex;
        currentSampleIndex = sampleIndex % samples.size();
        ofLog() << "switching to sample index curr " << currentSampleIndex;
        currentSampleIndex >> sampler.in_select();
    }


    pdsp::TriggerControl trig;

    pdsp::ParameterAmp   damp;

    ofParameterGroup parameters;
    pdsp::Parameter     timeControl;
    pdsp::Parameter     densityControl;
    pdsp::Parameter     dampingControl;
    pdsp::Parameter     hiCutControl;
    pdsp::Parameter     modFreqControl;
    pdsp::Parameter     modAmountControl;
    pdsp::Parameter     lowCutControl;
    pdsp::ParameterGain	verbGain;

    pdsp::Parameter     pitchControl;

    size_t     currentSampleIndex = 0;

    pdsp::BasiVerb reverb;
    pdsp::PatchNode     triggers;
    pdsp::Sampler       sampler;
    pdsp::AHR           env;
    pdsp::Amp           amp;
    pdsp::LowCut        lowcut;

    //pdsp::SampleBuffer sample;
    std::vector<pdsp::SampleBuffer*> samples;

    pdsp::Delay         delay;
    pdsp::ParameterGain delayGain;
    pdsp::Parameter     delayTimeControl;
    pdsp::Parameter     delayFeedbackControl;
    pdsp::Parameter     delayDampControl;

    pdsp::ParameterGain fader;

    ofParameterGroup ui; // for debugging purposes
};
