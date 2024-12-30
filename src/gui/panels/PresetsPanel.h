#pragma once

#include "EsenciaPanelBase.h"
#include "PresetsManager.h"

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

	PresetManager presetManager;
	std::vector<ParametersBase*> allParameters;


public:
		ofParameter<string> curPreset;
		int activePreset = 0;
		int prevPreset = 0;

	void setup(ofxGui& gui, PresetsParameters& preParams, SimulationParameters& simParams, CameraParameters& camParams, RenderParameters& renParams, PresetManager& presetMan) {
		// store references to all parameters to apply the presets data
		simulationParams = simParams;
		cameraParams = camParams;
		renderParams = renParams;
		presetParams = preParams;
		presetManager = presetMan;
		allParameters = { &simulationParams, &cameraParams, &renderParams };
		// TO-DO: 
		// refactor, get only the map


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
				ofJson({ {"width", 30}, {"height", 30}, {"border-width", 1}, {"type", "fullsize"}, {"text-align", "center"} , {"background-color", "#FFFFF33"} }));
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

	//void onSelectToggle(int& i) {

	//}

	//void mouseRel(ofMouseEventArgs& m) {
	//	ofLog() << "mouse released";
	//}

	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;
		
		// assign preset from the keyboard
		// SHIFT + n = 10 + n
		if (key >= '1' && key <= '9') {
			int index = key - '1' + 1; // convert to int by removing the ascii offset
			
			if (e.hasModifier(OF_KEY_SHIFT)) { 
				index += 10;
			}
			//if (index > 16) { index = 16; } // this is clamp by the setActivePreset

			setActivePreset(index);
		}
		else if (key == '0') {
			setActivePreset(10);
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

	/// <summary>
	/// Set the active preset ID (1-based)
	/// It will transform the index to 0-based to match the toggle index in the gui group
	/// </summary>
	/// <param name="i">consider 1 as the first preset</param>
	void setActivePreset(int i) {
		i = clamp(i, 1, 16);

		if (activePreset != i) {
			prevPreset = activePreset;
			activePreset = i;

			ofLog() << "Selecting preset slot " << i;
		}
		
		curPreset.set( std::to_string(i) );

		presetManager.applyPreset(i, allParameters);

		//i = clamp(i-1, 0, 15); // clamp between 0 and 15 for the 16 presets
		i--;
		presetToggles->setActiveToggle(i);
	}







	void savePreset() {
		ofLog() << "save pressed" ;
		presetManager.savePreset(activePreset, allParameters);
	}


	void clearPreset() {
		ofLog() << "clearing presets";
	}


	// button listeners
	///////////////////////////////////
	void presetButtonListener(bool& v) {
		if (v == true) {
			ofLog() << "preset button pressed, active: " << presetToggles->getActiveToggleIndex();
		}
		//curPreset.set(presetToggles->getActiveToggleIndex().toString());
		//ofLog() << "active preset: " << presetToggles->getActiveToggleIndex();
		return;
	}

	void saveButtonListener(bool& v) {
		savePreset();
	}
	
	void clearButtonListener(bool& v) {
		ofLog() << "clear pressed";
	}

	void copytoButtonListener(bool& v) {
		ofLog() << "copyTo pressed";
	}



	// update
	///////////////////////////////////
	void update() {
		if (activePreset != presetToggles->getActiveToggleIndex() + 1) {
			ofLog() << "applying active preset: " << presetToggles->getActiveToggleIndex();
			setActivePreset(presetToggles->getActiveToggleIndex() + 1);
		}

		presetManager.updateParameters(allParameters);
	}
};