#pragma once

#include "EsenciaPanelBase.h"
#include "PresetsManager.h"

class SequencePanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(11, 25, 12, 0);
	const ofColor &BG_COLOR = ofColor(100, 100, 100, 100);

	PresetManager *presetManager = nullptr;
	PresetsParameters *presetParams;

	ofParameter<bool> isPlaying;
	ofxGuiButton *playButton;

public:
	ofParameter<string> curPreset;

	void setup(ofxGui& gui, PresetsParameters *params, PresetManager &presetMan) {
		panel = gui.addPanel("sequence");

		presetParams = params;
		presetManager = &presetMan;

		panel->add<ofxGuiTextField>(presetParams->sequence.set("1,2,1"));

		playButton = panel->add<ofxGuiButton>(isPlaying.set("play", false),
			ofJson( {{"type", "fullsize"}, {"text-align","center"} }));

		playButton->addListener(this, &SequencePanel::playBtnListener);

		isPlaying.addListener(this, &SequencePanel::isPlayingListener);

		ofxGuiGroup* durations = panel->addGroup("durations (s)", ofJson({
			{"flex-direction", "column"},
			}));

		durations->add<ofxGuiFloatSlider>(presetParams->transitionDuration.set("transition", 5.0, 1.0, 120.0),
			ofJson({ {"precision", 0} }));
		durations->add<ofxGuiFloatSlider>(presetParams->presetDuration.set("preset", 20.0, 10.0, 3000.0),
			ofJson({ {"precision", 0}} ));

		presetParams->transitionDuration.addListener(this, &SequencePanel::transitionDurationChanged);

		configVisuals(PANEL_RECT, BG_COLOR);
	}


	void transitionDurationChanged(float& duration) {
		//ofLogNotice("transitionDurationChanged") << duration;
	}

	void playBtnListener() {
		ofLogVerbose("SequencePanel::playBtnListener") << "Play button pressed";

	}

	// TODO: Change this to a listener for the button, not the parameter
	void isPlayingListener(bool& b) {
		if (!b) return; // only listen to "click down" button event

		if (!presetManager->isPlayingSequence()) {
			playSequence();
			playButtonSetStop();
		}
		else {
			stopSequence();
			playButtonSetPlaying();
		}
	}

	void playButtonSetStop() {
		playButton->setLabel("playing");
		//playButton->setTextColor(ofColor::white);
		playButton->setBackgroundColor(ofColor(ofColor::lightSeaGreen,200));
		playButton->setNeedsRedraw();
	}
	void playButtonSetPlaying() {
		playButton->setLabel("stopped");
		//playButton->setTextColor(ofColor(100, 100, 100, 200));
		playButton->setBackgroundColor(ofColor(ofColor::orangeRed, 200));
		playButton->setNeedsRedraw();
	}


	void playSequence() {
		ofLogVerbose("SequencePanel::playBtn") << "Playing sequence";
		presetManager->loadSequence(presetParams->sequence);
		presetManager->playSequence(presetParams->presetDuration, presetParams->transitionDuration);
		presetParams->sequence = presetManager->removeInvalidCharacters(presetParams->sequence);
		isPlaying = true;
	}

	void stopSequence() {
		ofLogVerbose("SequencePanel::stopBtn") << "Stoping sequence";
		presetManager->stopSequence();
	}


	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;

		if (key == ' ') {
			//bool t = true;
			//isPlayingListener(t);
		}

		// TODO: add shortcut for call isPlayingListener(true)
	}

};

