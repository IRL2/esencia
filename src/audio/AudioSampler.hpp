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

        env.enableDBTriggering(-24.0f, 5.0f);
        setAHR(300.0f, 0.0f, 300.0f); 

        trig >> triggers;
        triggers >> sampler >> lowcut >> reverb >> verbGain >> amp;

        envGate >> env >> amp.in_mod();
        //envGate >> env2 >> amp.in_mod();

        envSmoothControl >> env.in_attack();
        envSmoothControl >> env.in_release();

        sampler >> delay >> delayGain >> amp;
        sampler >> amp;

        //damp >> amp.in_mod();

        amp >> filter >> fader; // output

        timeControl >> reverb.in_time();
        densityControl >> reverb.in_density();
        dampingControl >> reverb.in_damping();
        reverbHiCutControl >> reverb.in_hi_cut();
        modFreqControl >> reverb.in_mod_freq();
        modAmountControl >> reverb.in_mod_amount();

        lowCutControl >> lowcut.in_freq();

        pitchControl >> sampler.in_pitch();

        delayTimeControl * 600.0f >> delay.in_time();
        delayFeedbackControl >> delay.in_feedback();
        delayDampControl >> delay.in_damping();

        ui.setName("audio sampler");

        // lhf
        ui.add(lowCutControl.set("low cut freq filter", 100, 20, 1000));
        ui.add(envSmoothControl.set("fade ms", 0, 0, 1000));    

        // reverb
        ofParameterGroup* reverbGroup = new ofParameterGroup();
        reverbGroup->setName("reverb");
        ui.add(*reverbGroup);
        reverbGroup->add(verbGain.set("reverb gain", 9, -48, 12));
        reverbGroup->add(reverbHiCutControl.set("reverb high cut freq", 5000, 3000, 20000));
        reverbGroup->add(timeControl.set("rt60", 3.33f, 0.05f, 20.0f));
        reverbGroup->add(densityControl.set("density", 0.5f, 0.0f, 1.0f));
        reverbGroup->add(dampingControl.set("damping", 0.5f, 0.0f, 1.0f));
        reverbGroup->add(modFreqControl.set("mod speed (hz)", 0.2f, 0.01f, 1.25f));
        reverbGroup->add(modAmountControl.set("mod depth (ms)", 0.8f, 0.0f, 3.0f));

        // delay
        ofParameterGroup* delayGroup = new ofParameterGroup();
        delayGroup->setName("delay");
        ui.add(*delayGroup);
        delayGroup->add(delayGain.set("delay gain", -3, -48, 12));
        delayGroup->add(delayTimeControl.set("delay time", 0.3f, 0.0f, 10.0f));
        delayGroup->add(delayFeedbackControl.set("delay feedback", 0.3f, 0.0f, 0.95f));
        delayGroup->add(delayDampControl.set("delay damp", 0.5f, 0.0f, 1.0f));
        

        //filer
        ofParameterGroup* filterGroup = new ofParameterGroup();
        filterGroup->setName("filter");
        ui.add(*filterGroup);
        filterGroup->add(filterModeControl.set("mode", 0, 0, 6)); // lowpass, highpass, bandpass, notch, allpass
        //filterGroup->add(filerCutoffControl.set("cutoff freq", 54, 10, 120));
        filterGroup->add(filerResoControl.set("resonance", 0.0f, 0.0f, 1.0f));
        filterGroup->add(filerPitchControl.set("pitch", 120.0f, 0.0f, 120.0f));
        filterModeControl >> filter.in_mode();
        filerResoControl >> filter.in_reso();
        filerCutoffControl >> filter.in_cutoff();
        filerPitchControl >> filter.in_pitch();


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

    void setFilter(float mode = -1, float reso = -1, float pitch = -1) {
        if (mode > 0) filterModeControl.set(mode);
        if (reso > 0) filerResoControl.set(reso);
        if (pitch > 0) filerPitchControl.set(pitch);
    }

    void setReverb(float gain, float density, float time, float damping, float modFreq, float modAmount = NULL) {
        verbGain.set(gain);
        densityControl.set(density);
        timeControl.set(time);
        dampingControl.set(damping);
        //reverbHiCutControl.set(hiCut);
        modFreqControl.set(modFreq);
        modAmount ? modAmountControl.set(modAmount) : true;
    }

    void setDelay(float time, float feedback) {
        delayTimeControl.set(time);
        delayFeedbackControl.set(feedback);
    }

    void setDelay(float gain=-1, float time=-1, float feedback=-1, float damp=-1) {
        if (gain) delayGain.set(gain);
        if (time) delayTimeControl.set(time);
        if (feedback) delayFeedbackControl.set(feedback);
        if (damp) delayDampControl.set(damp);
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

        envGate.trigger(velocity);
        trig.trigger(velocity);
    }

    void stop() {
        //0.0f >> amp.in_mod(); // this works
        envGate.off();
    }

    void switchSampleIndex(int sampleIndex) {
        //ofLog() << "switching to sample index " << sampleIndex;
        currentSampleIndex = sampleIndex % samples.size();
        //ofLog() << "switching to sample index curr " << currentSampleIndex;
        currentSampleIndex >> sampler.in_select();
    }


    pdsp::TriggerControl trig;
    pdsp::TriggerControl envGate;

    pdsp::ParameterAmp   damp;

    ofParameterGroup parameters;
    pdsp::Parameter     timeControl;
    pdsp::Parameter     densityControl;
    pdsp::Parameter     dampingControl;
    pdsp::Parameter     reverbHiCutControl;
    pdsp::Parameter     modFreqControl;
    pdsp::Parameter     modAmountControl;
    pdsp::Parameter     lowCutControl;
    pdsp::ParameterGain	verbGain;
    pdsp::Parameter     filterModeControl;
    pdsp::Parameter     filerResoControl;
    pdsp::Parameter     filerCutoffControl;
    pdsp::Parameter     filerPitchControl;

    pdsp::Parameter     pitchControl;
    pdsp::Parameter     envSmoothControl;

    size_t     currentSampleIndex = 0;

    pdsp::BasiVerb reverb;
    pdsp::PatchNode     triggers;
    pdsp::Sampler       sampler;
    pdsp::Amp           preAmp;
    pdsp::AHR           env;
    pdsp::ADSR          env2;
    pdsp::Amp           amp;
    pdsp::LowCut        lowcut;
    pdsp::VAFilter      filter;

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
