#pragma once

#include "EsenciaPanelBase.h"
#include "ofxPresets.h"

class PresetsPanel : public EsenciaPanelBase {

	const ofRectangle PANEL_RECT = ofRectangle(10, 21, 6, 0);
	const ofColor& BG_COLOR = ofColor(100, 100, 100, 100);


	const float DEFAULT_TRANSITION_DURATION = 0.3f;

	ofxGuiButton* saveButton;
	ofxGuiButton* clearButton;
	ofxGuiButton* mutateButton;
	ofxGuiGroup* presetToggles;

	PresetsParameters* presetParams;
	ofxPresets* presetManager = nullptr;
	std::vector<ofxPresetsParametersBase*> allParameters;

	// button parameters
	ofParameter<bool> saveParam;
	ofParameter<bool> clearParam;
	ofParameter<bool> copyToParam;
	ofParameter<bool> mutateParam;

	float prevTransitionDuration = 0;

public:
    static const int MAX_PRESETS = 20; 
	ofxGuiToggle* statesButtons[MAX_PRESETS];
	ofParameter<string> curPreset;
	int activePreset = 1;
	int prevPreset = 1;


	void setup(ofxGui& gui, PresetsParameters* presetParamsReference, ofxPresets& presetsManagerReference) {

		presetParams = presetParamsReference;
		presetManager = &presetsManagerReference;

		// presets panel to group the preset toggles
		panel = gui.addPanel("presets");


		// the preset toggles
		//-----------------------------------
		presetToggles = panel->addGroup("presetToggles", ofJson({
			{"flex-direction", "row"},
			{"padding", 10},
			{"flex-wrap", "wrap"},
			{"width", 6 * 35}
			}));
		presetToggles->setShowHeader(false);

		for (int i = 0; i < MAX_PRESETS; i++) {
			statesButtons[i] = presetToggles->add<ofxGuiToggle>(presetParams->states[i].set(ofToString(i + 1), false),
				ofJson({ {"width", 30}, {"height", 30}, {"type", "fullsize"}, {"text-align", "center"} }));
		}

		recolorExistentPresetButtons();
		recolorPresetButtons();

		presetToggles->setExclusiveToggles(true);
		presetToggles->setActiveToggle(0);

		// Listener for the active toggle on the group -> should trigger when user clicks on any toggle
		presetToggles->getActiveToggleIndex().addListener(this, &PresetsPanel::onToggleGroupChangedActive);

		// Listener for when the preset starts applying a preset -> should trigger after any inputmode on preset and sequence change
		ofAddListener(presetManager->presetAppicationStarted, this, &PresetsPanel::onPresetmanagerApplyPreset);

		// Listener for when the preset finishes applying a preset -> should trigger on preset and sequence change
		ofAddListener(presetManager->transitionFinished, this, &PresetsPanel::onPresetmanagerTransitionFinished);


		// action buttons
		//-----------------------------------
		ofxGuiGroup* actionsPanel = panel->addGroup("actionsPanel");
		actionsPanel->setShowHeader(false);

		saveButton = actionsPanel->add<ofxGuiButton>(saveParam.set("save", false),
			ofJson({ {"type", "fullsize"}, {"text-align","center"} }));
		clearButton = actionsPanel->add<ofxGuiButton>(clearParam.set("clear", false),
			ofJson({ {"type", "fullsize"}, {"text-align","center"} }));
		mutateButton = actionsPanel->add<ofxGuiButton>(mutateParam.set("mutate", false),
			ofJson({ {"type", "fullsize"}, {"text-align","center"} }));

		//presetParams.save.enableEvents();
		saveParam.addListener(this, &PresetsPanel::saveButtonListener);
		clearParam.addListener(this, &PresetsPanel::clearButtonListener);
		mutateParam.addListener(this, &PresetsPanel::mutateButtonListener);

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
	/// (Attempt to) Updated the background color of the active preset button
	/// </summary>
	void recolorPresetButtons() {
		recolorExistentPresetButtons();

		for (int i = 0; i < MAX_PRESETS; i++) {
			if (i + 1 == abs(activePreset)) {
				recolorActiveButton(i);

				if (presetManager->presetExist(i + 1)) {
					recolorInexistentActiveButton(i);
				}
			}	
		}
	}

	/// <summary>
	/// Updates color of the preset buttons according to the existence of the preset
	/// </summary>
	void recolorExistentPresetButtons() {
		for (int i = 0; i < MAX_PRESETS; i++) {
			recolorResetButton(i);

			if (presetManager->presetExist(i + 1)) {  // exist
				recolorExistentButton(i);
			}

			statesButtons[i]->setNeedsRedraw();
		}
	}

	void recolorResetButton(int i) {
		statesButtons[i]->setTextColor(ofColor(20, 20, 20, 100));
		statesButtons[i]->setBackgroundColor(ofColor(200, 200, 200, 10));
		statesButtons[i]->setBorderColor(ofColor(20, 20, 20, 100));
		statesButtons[i]->setBorderWidth(1);
	}
	void recolorActiveButton(int i) {
		statesButtons[i]->setTextColor(ofColor(20, 20, 20, 100));
		statesButtons[i]->setBackgroundColor(ofColor(100, 100, 100, 200));
		statesButtons[i]->setBorderColor(ofColor(20, 20, 20, 100));
		statesButtons[i]->setBorderWidth(1);
	}
	void recolorExistentButton(int i) {
		statesButtons[i]->setTextColor(ofColor(20, 20, 20, 100));
		statesButtons[i]->setBackgroundColor(ofColor(ofColor::lightSeaGreen, 100));
		statesButtons[i]->setBorderColor(ofColor(ofColor::lightSeaGreen, 100));
		statesButtons[i]->setBorderWidth(15);
	}
	void recolorInexistentActiveButton(int i) {
		statesButtons[i]->setTextColor(ofColor::white);
		statesButtons[i]->setBackgroundColor(ofColor(ofColor::lightSeaGreen, 200));
		statesButtons[i]->setBorderColor(ofColor(ofColor::lightSeaGreen, 200));
		statesButtons[i]->setBorderWidth(15);
	}

	/// <summary>
	/// Set the active preset ID (1-based)
	/// It will transform the index to 0-based to match the toggle index in the gui group
	/// </summary>
	/// <param name="i">consider 1 as the first preset</param>
	void applyPreset(int i) {
		i = clamp(i, 1, MAX_PRESETS);

		if (activePreset != i) {
			prevPreset = activePreset;
			activePreset = i;

			ofLog(OF_LOG_NOTICE) << "PresetsPanel::applyPreset:: Selecting preset: " << i;
		}
		
		prevTransitionDuration = presetManager->interpolationDuration;
		presetManager->applyPreset(activePreset, DEFAULT_TRANSITION_DURATION);
	}



	void onPresetmanagerTransitionFinished() {
		if (!presetManager->isPlayingSequence()) {
			presetManager->interpolationDuration.set(prevTransitionDuration);
		}
		recolorPresetButtons();
	}



	void updateTogglesWithActivePreset() {
		presetToggles->setActiveToggle(activePreset - 1);
		recolorPresetButtons();
	}




	void savePreset() {
		presetManager->savePreset(activePreset);
		recolorPresetButtons();
		//recolorExistentPresetButtons();
	}


	void clearPreset() {
		presetManager->deletePreset(activePreset);
		//recolorExistentPresetButtons();
		recolorPresetButtons();
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
		if (v+1 == activePreset) return;

		if (!presetManager->isPlayingSequence()) {
			ofLog(OF_LOG_VERBOSE) << "PresetsPanel::onToggleGroupChangedActive:: Listener receives button ID pressed " << v;
			applyPreset(v + 1);
		}
		recolorPresetButtons();
	}

	/// <summary>
	/// Only for when the preset sequence is changing the active preset
	/// </summary>
	void onPresetmanagerApplyPreset() {
		if (presetManager->isPlayingSequence()) {
			presetToggles->setActiveToggle(presetManager->getCurrentPreset() - 1);
			activePreset = presetManager->getCurrentPreset();
		}
		recolorPresetButtons();
	}

	void saveButtonListener(bool& v) {
		savePreset();
	}
	
	void clearButtonListener(bool& v) {
		clearPreset();
	}

	void mutateButtonListener(bool& v) {
		presetManager->mutate();
	}


	// update
	///////////////////////////////////
	void update() {
		presetManager->update();
	}


};
