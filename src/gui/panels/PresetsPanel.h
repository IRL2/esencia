#pragma once

#include "EsenciaPanelBase.h"

class PresetsPanel : public EsenciaPanelBase {

	// default initial values
	const float PARTICLES_INITIAL = 120;
	const float PARTICLES_MIN = 1.0;
	const float PARTICLES_MAX = 200.0;

	const int RADIUS_INITIAL = 3;
	const int RADIUS_MIN = 1;
	const int RADIUS_MAX = 30;

	const int CIRCULAR_WIDTH = 180;
	const int CIRCULAR_HEIGHT = 180;
	const int RADIUS_HEIGHT = 50;

	const ofRectangle PANEL_RECT = ofRectangle(1, 22, 6, 0);
	const ofColor& BG_COLOR = ofColor(100, 100, 100, 100);

	SimulationParameters simulationParams;
	CameraParameters cameraParams; 
	RenderParameters renderParams;
	PresetsParameters presetParams;

	ofxGuiToggle *statesButtons[16];
	ofxGuiButton *saveButton;
	ofxGuiButton *clearButton;
	ofxGuiButton *copytoButton;
	ofxGuiGroup* presetToggles;



public:
		ofParameter<string> curPreset;

	void setup(ofxGui& gui, PresetsParameters& preParams, SimulationParameters& simParams, CameraParameters& camParams, RenderParameters& renParams) {
		// store references to all parameters to apply the presets data
		simulationParams = simParams;
		cameraParams = camParams;
		renderParams = renParams;
		presetParams = preParams;

		// create the panel
		// the panel has 2 groups: the toggle buttons, the actions buttons
		panel = gui.addPanel("presets");

		panel->add<ofxGuiLabel>(curPreset.set("presets",">"), ofJson({{"type", "label"}}));

		// the preset toggles (the group)
		presetToggles = panel->addGroup("presetToggles", ofJson({
			{"flex-direction", "row"},
			{"padding", 10},
			{"flex-wrap", "wrap"},
			{"width", 6 * 30}
			}));
		presetToggles->setShowHeader(false);

		for (int i = 0; i < 16; i++) {
			statesButtons[i] = presetToggles->add<ofxGuiToggle>(presetParams.states[i].set(ofToString(i + 1), false),
				ofJson({ {"width", 30}, {"height", 30}, {"border-width", 1}, {"type", "fullsize"}, {"text-align", "center"} , {"backgrround-color", "#FFFFF33"} }));
		}

		presetToggles->setExclusiveToggles(true);
		presetToggles->setActiveToggle(0);
		//presetParams.states[0].addListener(this, &PresetsPanel::presetButtonListener);



		// action buttons
		ofxGuiGroup* actions = panel->addGroup("actions");
		actions->setShowHeader(false);

		saveButton = actions->add<ofxGuiButton>(presetParams.save.set("save", false),
			ofJson({ {"type", "fullsize"} }));
		clearButton = actions->add<ofxGuiButton>(presetParams.clear.set("clear", false),
			ofJson({ {"type", "fullsize"} }));
		copytoButton = actions->add<ofxGuiButton>(presetParams.copyTo.set("copyTo", false),
			ofJson({ {"type", "fullsize"} }));

		presetParams.save.enableEvents();
		presetParams.save.addListener(this, &PresetsPanel::saveButtonListener);
		presetParams.clear.addListener(this, &PresetsPanel::clearButtonListener);
		presetParams.copyTo.addListener(this, &PresetsPanel::copytoButtonListener);



		configVisuals(PANEL_RECT, BG_COLOR);
	}

	void onSelectToggle(int& i) {

	}

	void mouseRel(ofMouseEventArgs& m) {
		ofLog() << "mouse released";
	}

	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;
		
		// assign preset from the keyboard
		// SHIFT + n = 10 + n
		if (key >= '1' && key <= '9') {
			int index = key - '1';
			
			if (e.hasModifier(OF_KEY_SHIFT)) { 
				index += 10;
			}
			if (index > 16) { index = 15; }

			presetToggles->setActiveToggle(index);
		}
		else if (key == '0') {
			presetToggles->setActiveToggle(9);
		}

		else if (key == 'S') {
			savePreset();
		}
		else if (key == 'L') {
			clearPreset();
		}
		else if (key == ' ') {
			ofLog() << "> active preset: " << presetToggles->getActiveToggleIndex();
			curPreset.set(presetToggles->getActiveToggleIndex().toString());
		}
	
	}


	void presetButtonListener(bool& v) {
		//curPreset.set(presetToggles->getActiveToggleIndex().toString());
		//ofLog() << "active preset: " << presetToggles->getActiveToggleIndex();
		return;
	}


	void savePreset() {
		ofLog() << "save pressed";
	}
	void saveButtonListener(bool& v) {
		savePreset();
	}



	void clearButtonListener(bool& v) {
		ofLog() << "clear pressed";
	}
	void clearPreset() {
		ofLog() << "clearing presets";
	}


	void copytoButtonListener(bool& v) {
		ofLog() << "copyTo pressed";
	}



	void update() {
		curPreset.set(presetToggles->getActiveToggleIndex().toString());
	}
};