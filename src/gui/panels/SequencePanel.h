#pragma once

#include "EsenciaPanelBase.h"
#include "PresetsManager.h"

class SequencePanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(11, 25, 8, 0);
	const ofColor &BG_COLOR = ofColor(100, 100, 100, 100);

	PresetManager presetManager;
	PresetsParameters *presetParams;

	ofParameter<bool> isPlaying;
	ofxGuiButton *playButton;

public:
	ofParameter<string> curPreset;

	void setup(ofxGui& gui, PresetsParameters *params, PresetManager& presetMan) {
		panel = gui.addPanel("sequence");

		presetParams = params;
		//presetManager = presetMan;

		panel->add<ofxGuiTextField>(presetParams->sequence.set("1-4"));
		playButton = panel->add<ofxGuiButton>(isPlaying.set("play", false),
			ofJson( {{"type", "fullsize"}}));
		playButton->addListener(this, &SequencePanel::playPressed);

		ofxGuiGroup* durations = panel->addGroup("durations (s)", ofJson({
			{"flex-direction", "column"},
			}));

		durations->add<ofxGuiFloatSlider>(presetParams->transitionDuration.set("transition", 1.0, 0.0, 120.0),
			ofJson({ {"precision", 0} }));
		durations->add<ofxGuiFloatSlider>(presetParams->presetDuration.set("preset", 120.0, 1.0, 30000.0),
			ofJson({ {"precision", 0}} ));

		presetParams->transitionDuration.addListener(this, &SequencePanel::transitionDurationChanged);

		configVisuals(PANEL_RECT, BG_COLOR);
	}


	void transitionDurationChanged(float& duration) {
		ofLogNotice("transitionDurationChanged") << duration;
	}

	void playPressed() {
		isPlaying = !isPlaying;
	}


	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;
	}

};

