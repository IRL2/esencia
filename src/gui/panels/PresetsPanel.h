#pragma once

#include "EsenciaPanelBase.h"
#include "PresetsManager.h"

class PresetsPanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(1, 22, 6, 0);
	const ofColor& BG_COLOR = ofColor(100, 100, 100, 100);

	const float DEFAULT_TRANSITION_DURATION = 2.0;

	SimulationParameters simulationParams;
	CameraParameters cameraParams; 
	RenderParameters renderParams;
	PresetsParameters *presetParams;

	ofxGuiToggle *statesButtons[16];
	ofxGuiButton *saveButton;
	ofxGuiButton *clearButton;
	ofxGuiButton *copytoButton;
	ofxGuiGroup *presetToggles;

	PresetManager *presetManager = nullptr;
	std::vector<ParametersBase*> allParameters;
	
	// button parameters
	ofParameter<bool> saveParam;
	ofParameter<bool> clearParam;
	ofParameter<bool> copyToParam;



public:
		ofParameter<string> curPreset;
		int activePreset = 0;
		int prevPreset = 0;


	// TODO: review if this pointers or references are needed and correct

	void setup(ofxGui &gui, PresetsParameters *preParams, PresetManager &presetMan, SimulationParameters &simParams, CameraParameters &camParams, RenderParameters &renParams) {
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
			{"width", 6 * 30}
			}));
		presetToggles->setShowHeader(false);

		for (int i = 0; i < 16; i++) {
			statesButtons[i] = presetToggles->add<ofxGuiToggle>(presetParams->states[i].set(ofToString(i + 1), false),
				ofJson({ {"width", 30}, {"height", 30}, {"border-width", 1}, {"type", "fullsize"}, {"text-align", "center"} }));
		}

		recolorPresetButtons();

		presetToggles->setExclusiveToggles(true);
		presetToggles->setActiveToggle(0);
		//presetParams.states[0].addListener(this, &PresetsPanel::presetButtonListener); // not using listener for the toggles, bc it triggers n-times toggles


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


		camParams.gaussianBlur.addListener(this, &PresetsPanel::onGaussianblurUpdate);

		configVisuals(PANEL_RECT, BG_COLOR);
	}


	// gaussian blur needs to be an odd value
	void onGaussianblurUpdate(int &value) {
		if (value % 2 == 0.0) {
			value = value + 1;
		}
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

			setActivePreset(index);
			//if (presetParams.copyTo.get() == true) {
			//	presetManager.clonePresetTo(activePreset, index, allParameters);
			//	presetParams.copyTo.set(false);
			//}
			//else {
			//}
		}
		else if (key == '0') {
			setActivePreset(10);
		}

		else if (key == 'S' && e.hasModifier(OF_KEY_CONTROL)) {
			savePreset();
		}
		else if (key == 'C' && e.hasModifier(OF_KEY_CONTROL)) {
			clearPreset();
		}
		//else if (key == 'C' && e.hasModifier(MOD_SHIFT)) {
		//	armCopytoPreset();
		//}
	}


	/// <summary>
	/// Updates colors on preset buttons according to the existence of the preset
	/// </summary>
	void recolorPresetButtons() {
		for (int i = 0; i < 16; i++) {
			if (presetManager->presetExist(i + 1)) {
				statesButtons[i]->setBackgroundColor(ofColor(100, 100, 100, 200));
				statesButtons[i]->setTextColor(ofColor(255, 255, 255, 255));
			}
			else {
				statesButtons[i]->setTextColor(ofColor(20, 20, 20, 100));
				statesButtons[i]->setBackgroundColor(ofColor(200, 200, 200, 10));
			}
			statesButtons[i]->setNeedsRedraw();
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

			ofLog() << "PresetsPanel::setActivePreset:: Selecting preset: " << i;
		}
		
		presetManager->applyPreset(activePreset, DEFAULT_TRANSITION_DURATION);

		i--;
		presetToggles->setActiveToggle(i);

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


	// button listeners
	///////////////////////////////////
	void presetButtonListener(bool& v) {
		if (v == true) {
			ofLog() << "PresetsPanel::presetButtonListener:: Preset button pressed: " << presetToggles->getActiveToggleIndex();
		}
		//curPreset.set(presetToggles->getActiveToggleIndex().toString());
		//ofLog() << "active preset: " << presetToggles->getActiveToggleIndex();
		return;
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
		if (activePreset != presetToggles->getActiveToggleIndex() + 1) {
			setActivePreset(presetToggles->getActiveToggleIndex() + 1);
			ofLogNotice("PresetsPanel::update") << "Updating active preset to follow the GUI" << activePreset;
		}

		presetManager->update();

		// to sync the gui with the activePreset
		if (presetManager->isPlayingSequence()) {
			if (presetToggles->getActiveToggleIndex() + 1 != presetManager->getCurrentPreset()) {
				presetToggles->setActiveToggle(presetManager->getCurrentPreset() - 1);
				ofLogNotice("PresetsPanel::update") << "Updating the preset GUI to follow the active preset from the manager" << activePreset;
			}
		}
	}
};
