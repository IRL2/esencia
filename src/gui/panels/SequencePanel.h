#pragma once

#include "EsenciaPanelBase.h"
#include "ofxPresets.h"

class SequencePanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(11, 25, 12, 0);
	const ofColor &BG_COLOR = ofColor(100, 100, 100, 100);

	const float DEFAULT_TRANSITION_DURATION_INIT = 5.0;
	const float DEFAULT_TRANSITION_DURATION_MAX = 10.0;
	const float DEFAULT_TRANSITION_DURATION_MIN = 0.5;
	const float DEFAULT_PRESET_DURATION_INIT = 10.0;
	const float DEFAULT_PRESET_DURATION_MAX = 300.0;
	const float DEFAULT_PRESET_DURATION_MIN = 10.0;

	const std::string DEFAULT_SEQUENCE = "?";

	ofxPresets *presetManager = nullptr;
	PresetsParameters*presetParams;

	ofParameter<bool> isPlaying;
	ofxGuiButton *playButton;

public:
	ofParameter<string> curPreset;

	void setup(ofxGui& gui, PresetsParameters *params, ofxPresets&presetMan) {
		panel = gui.addPanel("sequence");

		presetParams = params;
		presetManager = &presetMan;

		panel->add<ofxGuiTextField>(presetParams->sequence.set(DEFAULT_SEQUENCE));

		playButton = panel->add<ofxGuiButton>(isPlaying.set("play", false),
			ofJson( {{"type", "fullsize"}, {"text-align","center"} }));

		playButton->addListener(this, &SequencePanel::playBtnListener);

		isPlaying.addListener(this, &SequencePanel::isPlayingListener);

		ofxGuiGroup* durations = panel->addGroup("durations (s)", ofJson({
			{"flex-direction", "column"},
			}));

		// TODO: Cant use the precision parameter directly, bc it will mess with preset panel sends an applyPreset with default interpol value
		durations->add<ofxGuiFloatSlider>(
			//presetParams->transitionDuration.set("interpolation", DEFAULT_TRANSITION_DURATION_INIT, DEFAULT_TRANSITION_DURATION_MIN, DEFAULT_TRANSITION_DURATION_MAX),
			presetManager->interpolationDuration.set("interpolation", DEFAULT_TRANSITION_DURATION_INIT, DEFAULT_TRANSITION_DURATION_MIN, DEFAULT_TRANSITION_DURATION_MAX),
			ofJson({ {"precision", 0} }));

		durations->add<ofxGuiFloatSlider>(
			presetManager->sequencePresetDuration.set("preset", DEFAULT_PRESET_DURATION_INIT, DEFAULT_PRESET_DURATION_MIN, DEFAULT_PRESET_DURATION_MAX),
			ofJson({ {"precision", 0}} ));

		//presetParams->transitionDuration.addListener(this, &SequencePanel::transitionDurationChanged);
		updatePlaybuttonToStopped();

		configVisuals(PANEL_RECT, BG_COLOR);
	}


	// not used
	void transitionDurationChanged(float& duration) {
		if (presetManager->isPlayingSequence()) {
			presetManager->interpolationDuration.set(duration);
		}
		//ofLogNotice("transitionDurationChanged") << duration;
	}

	void playBtnListener() {
		ofLogVerbose("SequencePanel::playBtnListener") << "Play button pressed";
	}

	// TODO: Change this to a listener for the button, not the parameter
	void isPlayingListener(bool& b) {
		ofLog() << "isPlayingListener: " << b;
		if (!b) return; // only listen to "click down" button event
		
		togglePlayStop();
	}

	void togglePlayStop() {
		if (!presetManager->isPlayingSequence()) {
			playSequence();
		}
		else {
			stopSequence();
		}
	}


	void updatePlaybuttonToPlaying() {
		playButton->setLabel("playing");
		playButton->setBackgroundColor(ofColor(ofColor::lightSeaGreen,200));
		playButton->setNeedsRedraw();
	}
	void updatePlaybuttonToStopped() {
		playButton->setLabel("stopped");
		playButton->setBackgroundColor(ofColor(ofColor::orangeRed, 200));
		playButton->setNeedsRedraw();
	}


	void playSequence() {
		ofLogVerbose("SequencePanel::playBtn") << "Playing sequence";
		presetManager->loadSequence(presetParams->sequence);
		presetManager->playSequence(presetManager->sequencePresetDuration, presetManager->interpolationDuration);
		//presetManager->interpolationDuration = presetParams->transitionDuration;
		presetManager->removeInvalidCharacters(presetParams->sequence);

		//isPlaying = true; // this is done by the listener
		updatePlaybuttonToPlaying();
	}

	void stopSequence() {
		ofLogVerbose("SequencePanel::stopBtn") << "Stoping sequence";
		presetManager->stop();
		//isPlaying = false; // this is done by the listener
		updatePlaybuttonToStopped();
	}


	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;

		if (key == ' ') {
			togglePlayStop();
		}
	}

};

