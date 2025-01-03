#pragma once

#include "EsenciaPanelBase.h"
#include "PresetsManager.h"

class PresetsPanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(1, 22, 6, 0);
	const ofColor& BG_COLOR = ofColor(100, 100, 100, 100);

	SimulationParameters simulationParams;
	CameraParameters cameraParams; 
	RenderParameters renderParams;
	PresetsParameters *presetParams;

	ofxGuiToggle *statesButtons[16];
	ofxGuiButton *saveButton;
	ofxGuiButton *clearButton;
	ofxGuiButton *copytoButton;
	ofxGuiGroup *presetToggles;

	PresetManager *presetManager;
	std::vector<ParametersBase*> allParameters;
	
	// button parameters
	ofParameter<bool> saveParam;
	ofParameter<bool> clearParam;
	ofParameter<bool> copyToParam;



public:
		ofParameter<string> curPreset;
		int activePreset = 0;
		int prevPreset = 0;



	void setup(ofxGui& gui, PresetsParameters *preParams, SimulationParameters& simParams, CameraParameters& camParams, RenderParameters& renParams, PresetManager& presetMan) {
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

		presetToggles->setExclusiveToggles(true);
		presetToggles->setActiveToggle(0);
		//presetParams.states[0].addListener(this, &PresetsPanel::presetButtonListener); // not using listener for the toggles, bc it triggers n-times toggles


		// action buttons
		///////////////////////////////////
		ofxGuiGroup* actions = panel->addGroup("actions");
		actions->setShowHeader(false);

		saveButton = actions->add<ofxGuiButton>(saveParam.set("save", false),
			ofJson({ {"type", "fullsize"} }));
		clearButton = actions->add<ofxGuiButton>(clearParam.set("clear", false),
			ofJson({ {"type", "fullsize"} }));
		//copytoButton = actions->add<ofxGuiButton>(presetParams.copyTo.set("copyTo", false),
		//	ofJson({ {"type", "fullsize"} }));

		//presetParams.save.enableEvents();
		saveParam.addListener(this, &PresetsPanel::saveButtonListener);
		clearParam.addListener(this, &PresetsPanel::clearButtonListener);
		copyToParam.addListener(this, &PresetsPanel::copytoButtonListener);


		camParams.gaussianBlur.addListener(this, &PresetsPanel::onGaussianblurUpdate);

		//preParams.transitionDuration.set("trans", 5.0, 0.0, 120.0);

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

		else if (key == 'S') {
			savePreset();
		}
		else if (key == 'D') {
			clearPreset();
		}
		else if (key == 'C') {
			armCopytoPreset();
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

			ofLog() << "PresetsPanel::setActivePreset:: Selecting preset slot " << i;
		}
		
		ofLog() << "PresetsPanel::setActivePreset:: Applying preset with duration " << presetParams->transitionDuration.get();
		presetManager->applyPreset(activePreset, presetParams->transitionDuration.get());

		i--;
		presetToggles->setActiveToggle(i);
	}







	void savePreset() {
		presetManager->savePreset(activePreset);
		statesButtons[activePreset]->setAttribute("background-color", "#FF0066"); // TODO: not working
	}


	void clearPreset() {
		presetManager->deletePreset(activePreset);
		//statesButtons[activePreset-1]->loadConfig(ofJson({ {"background-color", "#000000"} }));
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
		if (activePreset != presetToggles->getActiveToggleIndex() + 1) {
			setActivePreset(presetToggles->getActiveToggleIndex() + 1);
			ofLog() << "PresetsPanel::update:: Updating active preset to " << activePreset;
		}

		//presetManager.updateParameters(allParameters);
		presetManager->update();
	}
};
