
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
//#include "ofxGuiExtended.h"

// datatable based polysynth

class PolySynth {

public:









    class Voice : public pdsp::Patchable {
        friend class PolySynth;

    public:
        Voice() {}
        Voice(const Voice& other) {}

        float PolySynth::Voice::meter_mod_env() const {
            return envelope.meter_output();
        }

        //float PolySynth::Voice::meter_pitch() const {
        //    return oscillator.meter_pitch();
        //}

        // m brings the instrument context/parameters
        void PolySynth::Voice::setup(PolySynth& m, int v) {

            addModuleInput("trig", voiceTrigger);
            addModuleInput("pitch", oscillator.in_pitch());
            //addModuleInput("fm", oscillator.in_fm());
            addModuleOutput("signal", amp);

            // SIGNAL PATH
            oscillator >> filter >> amp >> m.leakDC;
            //2.0f >> oscillator.in_ratio();
            //pdsp::CheapPulse sw;
            //sw  >> oscillator;

            individualTrigger >> voiceTrigger;
            internalPitchCtrl >> oscillator.in_pitch();

            // MODULATIONS AND CONTROL
            voiceTrigger >> envelope >> amp.in_mod();
            envelope >> m.env_filter_amt.ch(v) >> filter.in_pitch();
            m.lfo_filter_amt >> filter.in_pitch();
            m.cutoff_ctrl >> filter.in_pitch();
            m.reso_ctrl >> filter.in_reso();
            m.filter_mode_ctrl >> filter.in_mode();

            m.env_attack_ctrl >> envelope.in_attack();
            m.env_decay_ctrl >> envelope.in_decay();
            m.env_sustain_ctrl >> envelope.in_sustain();
            m.env_release_ctrl >> envelope.in_release();
        }

        float PolySynth::Voice::getCurrentPitch() {
            return oscillator.meter_pitch();
        }

        pdsp::PatchNode     voiceTrigger;
        pdsp::TriggerControl individualTrigger;
        pdsp::Parameter     internalPitchCtrl;

    private:

        pdsp::VAOscillator oscillator;
        //pdsp::FMOperator        oscillator;
        pdsp::VAFilter          filter;
        pdsp::Amp               amp;


        pdsp::ADSR          envelope;
    }; 









    vector<Voice>       voices;
    ofParameterGroup    ui;
    vector<float> notes;

    pdsp::TriggerControl trigger;
    pdsp::Parameter     pitchCtrl;
    pdsp::ParameterGain gain;  // fader


    pdsp::Patchable& PolySynth::ch(int index) {
        index = index % 2;
        return gain.ch(index);
    }

    void PolySynth::setPitch(float pitch) {
        pitchCtrl.set(pitch);
    }

    void PolySynth::setLFOfreq(float freq) {
        //lfo_speed_ctrl.set(freq);
        lfo_filter_amt.set(freq);
    }

    void PolySynth::on(float v = 0.1f) {
        trigger.trigger(v);
    }
    void PolySynth::off() {
        trigger.off();
    }



    void PolySynth::playFor(float pitch, float duration, float velocity = 0.6f) {
        setPitch(pitch);
        on(velocity);

        // schedule the off event
        ofEventListener* listener = new ofEventListener();
        *listener = ofEvents().update.newListener([this, startTime = ofGetElapsedTimeMillis(), duration, listener](ofEventArgs&) {
            uint64_t currentTime = ofGetElapsedTimeMillis();
            uint64_t durationMs = static_cast<uint64_t>(duration);

            if (currentTime >= startTime + durationMs) {
                this->off();
                listener->unsubscribe();
            }
        });
    }


    void PolySynth::polyPlayFor(float pitch, float duration){

        ofLog() << ofToString(notes);

        int voiceId = -1;
        for (int i=0; i<static_cast<int>(voices.size()); i++){
            if (notes[i] == pitch || notes[i] == 0) {
                voiceId = i;
                ofLog() << "Using voice " << voiceId << " for pitch " << pitch << (notes[i] == pitch?" reuse ":"") << (notes[i] == 0 ? " empty " : "");
                break;
            }
        }
        if (voiceId == -1) return;


        notes[voiceId] = pitch;
        voices[voiceId].internalPitchCtrl.set(pitch);
        //trigger.trigger(1.0f);
        voices[voiceId].individualTrigger.trigger(1.0f - ((voiceId+2) / 10));

        ofLog() << ofToString(notes);


        ofEventListener* listener = new ofEventListener();
        *listener = ofEvents().update.newListener([this, voiceId, startTime = ofGetElapsedTimeMillis(), duration, listener](ofEventArgs&) {
            uint64_t currentTime = ofGetElapsedTimeMillis();
            uint64_t durationMs = static_cast<uint64_t>(duration);
            if (currentTime >= startTime + durationMs) {
                voices[voiceId].individualTrigger.off();
                notes[voiceId] = 0;
                listener->unsubscribe();
            }
        });
    }


    void PolySynth::setup(int numVoices) {
        voices.resize(numVoices);
        notes.resize(numVoices);

        for (int i = 0; i < numVoices; ++i) {
            voices[i].setup(*this, i);
            trigger >> voices[i].in("trig");
            pitchCtrl >> voices[i].in("pitch");
        }

        20.0f >> leakDC.in_freq(); // non audible bellow 20hz

        leakDC >> chorus.ch(0) >> gain.ch(0);
        leakDC >> chorus.ch(1) >> gain.ch(1);

        // pdsp::Switch EXAMPLE ---------------------------------------------------
        lfo_switch.resize(5);  // resize input channels
        lfo.out_triangle() >> lfo_switch.input(0); // you cannot use this input() method in a chain
        lfo.out_saw() >> lfo_switch.input(1); // because: API reasons
        lfo.out_square() >> lfo_switch.input(2);
        lfo.out_sine() >> lfo_switch.input(3);
        lfo.out_sample_and_hold() >> lfo_switch.input(4);

        lfo_wave_ctrl >> lfo_switch.in_select(); // input for output selection

        lfo_speed_ctrl >> lfo.in_freq();
        lfo_switch >> lfo_filter_amt;

        chorus_speed_ctrl >> chorus.in_speed();
        chorus_depth_ctrl >> chorus.in_depth();

        ui.setName("polysynth");
        // filter
        ui.add(filter_mode_ctrl.set("filter mode", 1, 0, 5));
        ui.add(cutoff_ctrl.set("filter cutoff", 54, 10, 120));
        ui.add(reso_ctrl.set("filter reso", 0.5f, 0.0f, 1.0f));
        // env
        ui.add(env_attack_ctrl.set("env attack", 400, 5, 1200));
        ui.add(env_decay_ctrl.set("env decay", 1200, 5, 1200));
        ui.add(env_sustain_ctrl.set("env sustain", 0.9f, 0.0f, 1.0f));
        ui.add(env_release_ctrl.set("env release", 2000, 5, 2000));
        ui.add(env_filter_amt.set("env to filter", 31, 0, 60));
        // lfo
        ui.add(lfo_wave_ctrl.set("lfo wave", 1, 0, 4));
        ui.add(lfo_speed_ctrl.set("lfo freq", 0.25f, 0.000f, 4.0f));
        ui.add(lfo_filter_amt.set("lfo to filter", 14, 0, 60));
        // chorus
        ui.add(chorus_speed_ctrl.set("chorus freq", 0.25f, 0.0f, 1.0f));
        ui.add(chorus_depth_ctrl.set("chorus depth", 10.0f, 1.0f, 10.0f));
        ui.add(gain.set("gain", -10, -48, 12));

        cutoff_ctrl.enableSmoothing(200.0f);
        gain.enableSmoothing(50.f);
    }



private: // --------------------------------------------------

    pdsp::Parameter     cutoff_ctrl;
    pdsp::Parameter     reso_ctrl;
    pdsp::Parameter     filter_mode_ctrl;

    pdsp::Parameter     env_attack_ctrl;
    pdsp::Parameter     env_decay_ctrl;
    pdsp::Parameter     env_sustain_ctrl;
    pdsp::Parameter     env_release_ctrl;
    pdsp::ParameterAmp  env_filter_amt;

    pdsp::Parameter     lfo_speed_ctrl;
    pdsp::Parameter     lfo_wave_ctrl;

    pdsp::LFO           lfo;
    pdsp::Switch        lfo_switch;
    pdsp::ParameterAmp  lfo_filter_amt;

    pdsp::LowCut			leakDC;

    // chorus ------------------------
    pdsp::DimensionChorus   chorus;
    ofParameterGroup    ui_chorus;
    pdsp::Parameter     chorus_speed_ctrl;
    pdsp::Parameter     chorus_depth_ctrl;
};

