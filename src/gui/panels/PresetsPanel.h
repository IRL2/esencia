#pragma once

#include "EsenciaPanelBase.h"
#include "ofxPresets.h"

class PresetsPanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(1, 21, 6, 0);
	const ofColor& BG_COLOR = ofColor(100, 100, 100, 100);

	const float DEFAULT_TRANSITION_DURATION = 0.5;

	SimulationParameters simulationParams;
	CameraParameters cameraParams; 
	RenderParameters renderParams;
	PresetsParameters *presetParams;

	ofxGuiToggle *statesButtons[16];
	ofxGuiButton *saveButton;
	ofxGuiButton *clearButton;
	ofxGuiButton *copytoButton;
	ofxGuiGroup *presetToggles;

	ofxPresets *presetManager = nullptr;
	std::vector<ofxPresetsParametersBase*> allParameters;
	
	// button parameters
	ofParameter<bool> saveParam;
	ofParameter<bool> clearParam;
	ofParameter<bool> copyToParam;



public:
		ofParameter<string> curPreset;
		int activePreset = 0;
		int prevPreset = 0;


	// TODO: review if this pointers or references are needed and correct

	void setup(ofxGui &gui, PresetsParameters *preParams, ofxPresets &presetMan, SimulationParameters &simParams, CameraParameters &camParams, RenderParameters &renParams) {
		// store references to all parameters to apply the presets data
		simulationParams = simParams;
		cameraParams = camParams;
		renderParams = renParams;
		presetParams = preParams;
		presetManager = &presetMan;
		allParameters = { &simulationParams, &renderParams };

		// create the panel with 2 groups: the toggle buttons, the actions buttons
		panel = gui.addPanel("presets");

		//panel->add<ofxGuiLabel>(curPreset.set("presets",">"), ofJson({{"type", "label"}}));

		// the preset toggles (the group)
		///////////////////////////////////
		presetToggles = panel->addGroup("presetToggles", ofJson({
			{"flex-direction", "row"},
			{"padding", 10},
			{"flex-wrap", "wrap"},
			{"width", 6 * 29}
			}));
		presetToggles->setShowHeader(false);

		for (int i = 0; i < 16; i++) {
			statesButtons[i] = presetToggles->add<ofxGuiToggle>(presetParams->states[i].set(ofToString(i + 1), false),
				ofJson({ {"width", 30}, {"height", 30}, {"type", "fullsize"}, {"text-align", "center"} }));
		}

		recolorPresetButtons();

		presetToggles->setExclusiveToggles(true);
		presetToggles->setActiveToggle(0);

		presetToggles->getActiveToggleIndex().addListener(this, &PresetsPanel::onToggleGroupChangedActive);
		//presetToggles->addEventListener(this, &PresetsPanel::onToggleGroupChangedActive);
		//presetParams.states[0].addListener(this, &PresetsPanel::onToggleGroupChangedActive); // not using listener for the toggles, bc it triggers n-times toggles
		ofAddListener(presetManager->presetAppicationStarted, this, &PresetsPanel::onPresetmanagerApplyPreset);

		// action buttons
		///////////////////////////////////
		ofxGuiGroup* actions = panel->addGroup("actions");
		actions->setShowHeader(false);

		saveButton = actions->add<ofxGuiButton>(saveParam.set("save", false),
			ofJson({ {"type", "fullsize"}, {"text-align","center"} }));
		clearButton = actions->add<ofxGuiButton>(clearParam.set("clear", false),
			ofJson({ {"type", "fullsize"}, {"text-align","center"} }));
		//copytoButton = actions->add<ofxGuiButton>(presetParams.copyTo.set("copyTo", false),
		//	ofJson({ {"type", "fullsize"} }));

		//presetParams.save.enableEvents();
		saveParam.addListener(this, &PresetsPanel::saveButtonListener);
		clearParam.addListener(this, &PresetsPanel::clearButtonListener);
		copyToParam.addListener(this, &PresetsPanel::copytoButtonListener);

		configVisuals(PANEL_RECT, BG_COLOR);
	}



	void keyReleased(ofKeyEventArgs& e) {
		int key = e.keycode;
		
		// assign preset from the keyboard
		// SHIFT + n = 10 + n
		if (key >= '1' && key <= '9') {
			int index = key - '1' + 1; // convert to int by removing the ascii offset
			
			if (e.hasModifier(OF_KEY_SHIFT)) { 
				index += 10;
			}

			onKeyboardSelectPreset(index);
		}
		else if (key == '0') {
			onKeyboardSelectPreset(10);
		}

		else if (key == 'S' && e.hasModifier(OF_KEY_CONTROL)) {
			savePreset();
		}
		else if (key == 'C' && e.hasModifier(OF_KEY_CONTROL)) {
			clearPreset();
		}
		else if (key == 'M') {
			presetManager->mutate();
		}
	}


	/// <summary>
	/// Updates colors on preset buttons according to the existence of the preset
	/// </summary>
	void recolorPresetButtons() {
		for (int i = 0; i < 16; i++) {
			if (presetManager->presetExist(i + 1)) {
				statesButtons[i]->setTextColor(ofColor(255, 255, 255, 255));
				statesButtons[i]->setBackgroundColor(ofColor(100, 100, 100, 200));
				ofLog() << "preset " << i + 1 << " exists";
			}
			else {
				statesButtons[i]->setTextColor(ofColor(20, 20, 20, 100));
				statesButtons[i]->setBackgroundColor(ofColor(200, 200, 200, 10));
			}
			if (i + 1 == presetManager->getCurrentPreset() ||
				i + 1 == activePreset) {
				statesButtons[i]->setTextColor(ofColor(255, 255, 255, 255));
				statesButtons[i]->setBackgroundColor(ofColor(ofColor::lightSeaGreen, 200));
				ofLog() << "preset " << i + 1 << " is " << (i + 1 == activePreset ? "local activePreset" : "manager current");
			}
			//statesButtons[i]->setNeedsRedraw();
		}
	}


	/// <summary>
	/// Set the active preset ID (1-based)
	/// It will transform the index to 0-based to match the toggle index in the gui group
	/// </summary>
	/// <param name="i">consider 1 as the first preset</param>
	void applyPreset(int i) {
		i = clamp(i, 1, 16);

		if (activePreset != i) {
			prevPreset = activePreset;
			activePreset = i;

			ofLog() << "PresetsPanel::applyPreset:: Selecting preset: " << i;
		}
		
		presetManager->applyPreset(activePreset, DEFAULT_TRANSITION_DURATION);

		i--;
		//presetToggles->setActiveToggle(i);

		//recolorPresetButtons();
	}



	void updateTogglesWithActivePreset() {
		presetToggles->setActiveToggle(activePreset - 1);
		recolorPresetButtons();
	}




	void savePreset() {
		presetManager->savePreset(activePreset);
		recolorPresetButtons();
	}


	void clearPreset() {
		presetManager->deletePreset(activePreset);
		recolorPresetButtons();
	}

	void armCopytoPreset() {
		ofLog() << "PresetsPanel::armCopytoPreset:: Arming copy to, from preset " << activePreset;
		//presetParams.copyTo.set(!presetParams.copyTo);
		copyToParam.set(true);
	}


	// listeners
	//-----------------------------------

	/// <summary>
	/// Called by the keyboard listener when numbers are pressed
	/// Not a listener but makes sense to be here
	/// </summary>
	/// <param name="id"></param>
	void onKeyboardSelectPreset(int id) {
		applyPreset(id);
		presetToggles->setActiveToggle(id-1);
		recolorPresetButtons();
	}

	/// <summary>
	/// May be called anytime the presetToggles changed the active toggle
	/// Should be called when a preset button is pressed on the GUI
	/// </summary>
	/// <param name="v"></param>
	void onToggleGroupChangedActive(int& v) {
		ofLog() << "PresetsPanel::onToggleGroupChangedActive:: Listener receives button ID pressed " << v;
		applyPreset(v + 1);
		recolorPresetButtons();
	}

	/// <summary>
	/// Only for when the preset sequence is changing the active preset
	/// </summary>
	void onPresetmanagerApplyPreset() {
		if (presetManager->isPlayingSequence()) {
			presetToggles->setActiveToggle(presetManager->getCurrentPreset() - 1);
		}
		//recolorPresetButtons();
	}

	void saveButtonListener(bool& v) {
		savePreset();
	}
	
	void clearButtonListener(bool& v) {
		clearPreset();
	}

	void copytoButtonListener(bool& v) {
		armCopytoPreset();
	}



	// update
	///////////////////////////////////
	void update() {
		// to sync the activePreset with the gui


		presetManager->update();
	}


};
