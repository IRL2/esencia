
#pragma once

#include "ofMain.h"
#include "ofxPDSP.h"
#include "ofxGuiExtended.h"

// datatable based polysynth

class DataSynth {

public:
    // class to rapresent each synth voice ------------
    class Voice : public pdsp::Patchable {
        friend class DataSynth;

    public:
        Voice() {}
        Voice(const Voice& other) {}

        float DataSynth::Voice::meter_mod_env() const {
            return envelope.meter_output();
        }

        float DataSynth::Voice::meter_pitch() const {
            return oscillator.meter_pitch();
        }

    private:
        void DataSynth::Voice::setup(DataSynth& m, int v) {

            addModuleInput("trig", voiceTrigger);
            addModuleInput("pitch", oscillator.in_pitch());
            addModuleOutput("signal", amp);

            oscillator.setTable( m.datatable );
            // SIGNAL PATH
            oscillator >> filter >> amp >> m.leakDC;

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

        pdsp::PatchNode     voiceTrigger;

        pdsp::DataOscillator    oscillator;
        pdsp::VAFilter          filter;
        pdsp::Amp               amp;


        pdsp::ADSR          envelope;
    }; // end voice class -----------------------------


    pdsp::DataTable  datatable;

    pdsp::Patchable& ch(int index);

    vector<Voice>       voices;
    ofParameterGroup    ui;

    void DataSynth::setup(int numVoices) {
        // -------------------------- PATCHING ------------------------------------
        voices.resize(numVoices);

        for (int i = 0; i < numVoices; ++i) {
            voices[i].setup(*this, i);
        }

        // we filter the frequency below 20 hz (not audible) just to remove DC offsets
        20.0f >> leakDC.in_freq();

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
        // ------------------------------------------------------------------------

        // CONTROLS ---------------------------------------------------------------
        ui.setName("DATASYNTH");

        ui.add(filter_mode_ctrl.set("filter mode", 0, 0, 5));
        ui.add(cutoff_ctrl.set("filter cutoff", 120, 10, 120));
        ui.add(reso_ctrl.set("filter reso", 0.0f, 0.0f, 1.0f));

        cutoff_ctrl.enableSmoothing(200.0f);

        ui.add(env_attack_ctrl.set("env attack", 50, 5, 1200));
        ui.add(env_decay_ctrl.set("env decay", 400, 5, 1200));
        ui.add(env_sustain_ctrl.set("env sustain", 1.0f, 0.0f, 1.0f));
        ui.add(env_release_ctrl.set("env release", 900, 5, 2000));
        ui.add(env_filter_amt.set("env to filter", 30, 0, 60));

        ui.add(lfo_wave_ctrl.set("lfo wave", 0, 0, 4));
        ui.add(lfo_speed_ctrl.set("lfo freq", 0.5f, 0.005f, 4.0f));
        ui.add(lfo_filter_amt.set("lfo to filter", 0, 0, 60));
        // ------------------------------------------------------------------------

        // Chorus -----------------------------------------------------------------
        chorus_speed_ctrl >> chorus.in_speed();
        chorus_depth_ctrl >> chorus.in_depth();
        ui.add(chorus_speed_ctrl.set("chorus freq", 0.5f, 0.25f, 1.0f));
        ui.add(chorus_depth_ctrl.set("chorus depth", 3.5f, 1.0f, 10.0f));
        ui.add(gain.set("gain", -9, -48, 12));
        gain.enableSmoothing(50.f);
        // ------------------------------------------------------------------------
    }



private: // --------------------------------------------------

    pdsp::ParameterGain gain;

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

    std::vector<float> partials_vector;
};

