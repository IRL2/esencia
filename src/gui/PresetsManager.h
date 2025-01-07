#pragma once

#include <fstream>
#include "ofJson.h"
#include "ofLog.h"
#include "parameters/ParametersBase.h"
#include "ofxEasing.h"


/*
* the json file follows the same structure as the parameters, with the first level being the group name

{
	"simulation": {
		"particles": 1000,
		"radius": 3,
		...
	},
	"render": {
		"param1": value,
		"param2": value,
		...
	}
}

The camera parameters are deliveritelly not stored in the presets,
this is because the camera is setup once per venue, so it should not be stored in the presets

TODO: Manipulate the map to include parameters on each module

TO-DO: Consider certaint camera parameters to be included in the presets. Maybe use a blacklist
*/


struct InterpolationData {
    float startTime;
    float duration;
    std::unordered_map<std::string, float> targetValues;
};


class PresetManager {
private:
    void saveParametersToJson(const std::string& jsonFilePath);
    void applyJsonToParameters(const std::string& jsonFilePath, float interpolationDuration);
    
    bool fileExist(const std::string& jsonFilePath);

    std::string sequenceString;
    std::vector<int> sequence;
	int sequenceIndex = 0;
    float sequenceTransitionDuration = 3.0f;
    float sequencePresetDuration = 20.0f;
    float lastUpdateTime = 0.0f;
	bool isTransitioning = false;  // flag to know if we are transitioning(interpolating) in the sequence

    std::unordered_map<std::string, InterpolationData> interpolationDataMap;

	std::vector<ParametersBase*>* params; // local reference to the parameters

    std::vector<int> parseSequence(std::string& input);
    std::vector<std::string> splitString(const std::string& str, char delimiter) const;

    std::string convertIDtoJSonFilename(int id);

public:
    void setup(std::vector<ParametersBase*>& parameters);
    void update();

    void applyPreset(int id, float duration);
    void savePreset(int id);
    void deletePreset(int id);
    void clonePresetTo(int from, int to);

    void updateParameters();
    
	void loadSequence(const std::string& sequenceString);
    void playSequence();
    void playSequence(float sequenceDuration, float transitionDuration);
    void stopSequence();

    int getCurrentPreset();
    void updateSequenceIndex();
    void onPresetFinished();
    void onTransitionFinished();

	bool isInterpolating() { return !interpolationDataMap.empty(); } // for when parameters are being interpolated

	bool isPlayingSequence() { return sequence.size() > 0; }

    bool presetExist(int id);

    static std::string removeInvalidCharacters(const std::string& input);

    ofEvent<void> presetFinishedEvent;
    ofEvent<void> transitionFinishedEvent;
};


void PresetManager::setup(std::vector<ParametersBase*>& parameters) {
	this->params = &parameters;
}



/// <summary>
/// Apply the values from a JSON file to the parameters
/// </summary>
/// <param name="jsonFilePath">Full path to the json file</param>
void PresetManager::applyJsonToParameters(const std::string& jsonFilePath, float interpolationDuration) {
    ofLog() << "PresetManager::applyJsonToParameters:: Applying JSON to parameters from " << jsonFilePath;

    // Read the JSON file
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        ofLogError() << "Could not open JSON file";
        return;
    }

    // Parse the JSON file
    ofJson j;
    file >> j;

    interpolationDataMap.clear();

    // Iterate over all items in the JSON
    for (auto& [group, v] : j.items()) {  // first level is the parameter group

        // Iterate over all parameter groups to find the corresponding 1st level set from the json
        for (auto& paramGroup : *params) {
            if (paramGroup->groupName == group) {

                InterpolationData interpolationData;
                interpolationData.startTime = ofGetElapsedTimef();
                interpolationData.duration = interpolationDuration; // Set the duration for interpolation

                // iterate over all items in the group
                for (auto& [key, value] : j[group].items()) {

                    // store actual param key and value
                    auto param = paramGroup->parameterMap[key];
                    if (param == nullptr) {
                        ofLog() << "Preset key " << key << " not found in " << group;
                        continue;
                    }

                    // cast to apply the value
                    const std::string paramTypeName = typeid(*param).name(); // get the type of the parameter to cast it

					try {
                        if (paramTypeName == typeid(ofParameter<bool>).name()) {
                            dynamic_cast<ofParameter<bool>*>(param)->set(value.get<bool>());
                        }
                        else if (paramTypeName == typeid(ofParameter<int>).name()) {
                            //dynamic_cast<ofParameter<int>*>(param)->set(value.get<int>());
                            interpolationData.targetValues[key] = value.get<int>();
                        }
                        else if (paramTypeName == typeid(ofParameter<float>).name()) {
                            //dynamic_cast<ofParameter<float>*>(param)->set(value.get<float>());
                            interpolationData.targetValues[key] = value.get<float>();
                        }
                        else if (paramTypeName == typeid(ofParameter<ofColor>).name()) {
                            ofColor color;
                            color.setHex(value.get<int>());
                            dynamic_cast<ofParameter<ofColor>*>(param)->set(color);
                        }
                    }
                    catch (const std::exception& e) {
                        ofLogError() << "Error applying value for key " << key << ": " << e.what();
                    }
                }

                interpolationDataMap[group] = interpolationData;
            }
        }
    }
}


/// <summary>
/// Apply a preset to the parameters
/// </summary>
/// <param name="id"></param>
/// <param name="parameterGroups"></param>
/// <param name="duration">interpolation diration</param>
void PresetManager::applyPreset(int id, float duration = 0.0f) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    if (fileExist(jsonFilePath)) {
        applyJsonToParameters(jsonFilePath, duration);
    }
    else {
        ofLog() << "PresetManager::applyPreset:: No json file for preset " << idStr;
    }
}


/// <summary>
/// public method to save the current parameters to a json file
/// </summary>
/// <param name="id">The preset ID (1-based)</param>
/// <param name="parameterGroups">all groups to be saved</param>
void PresetManager::savePreset(int id) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    saveParametersToJson(jsonFilePath);
}


std::string PresetManager::convertIDtoJSonFilename(int id) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
	return jsonFilePath;
}


/// <summary
/// Check if a file exists
/// </summary>
bool PresetManager::fileExist(const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        return false;
    }
    return true;
}

/// <summary>
/// Verify if an associated json file exists for a given preset ID
/// </summary>
/// <param name="id"></param>
/// <returns></returns>
bool PresetManager::presetExist(int id) {
    auto file = convertIDtoJSonFilename(id);
	return fileExist(file);
}



/// <summary>
/// Delete a preset file
/// </summary>
/// <param name="id"></param>
void PresetManager::deletePreset(int id) {
	std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
	std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
	if (fileExist(jsonFilePath)) {
		std::remove(jsonFilePath.c_str());
		ofLog() << "PresetManager::deletePreset" << "Preset " << idStr << " deleted";
	}
	//else {
	//	ofLog() << "PresetManager::deletePreset:: No json file for preset " << idStr;
	//}
}


/// <summary>
/// Take the given parameters from a vector and saves them as a json file with the parameters and save it to the parameters
/// </summary>
/// <param name="jsonFilePath"></param>
/// <param name="parameterGroups"></param>
void PresetManager::saveParametersToJson(const std::string& jsonFilePath) {
    ofLog() << "PresetManager::saveParametersToJson:: Saving JSON to esencia parameters to " << jsonFilePath;

    ofJson j;

    for (const auto& paramGroup : *params) {
        ofJson groupJson;
        for (const auto& [key, param] : paramGroup->parameterMap) {
			if (param != nullptr) { // nullptr means this parameter is the the group name
                const std::string paramTypeName = typeid(*param).name();

                try {
                    if (paramTypeName == typeid(ofParameter<bool>).name()) {
                        groupJson[key] = dynamic_cast<ofParameter<bool>*>(param)->get();
                    }
                    else if (paramTypeName == typeid(ofParameter<int>).name()) {
                        groupJson[key] = dynamic_cast<ofParameter<int>*>(param)->get();
                    }
                    else if (paramTypeName == typeid(ofParameter<float>).name()) {
                        groupJson[key] = dynamic_cast<ofParameter<float>*>(param)->get();
                    }
                    else if (paramTypeName == typeid(ofParameter<ofColor>).name()) {
                        ofColor color = dynamic_cast<ofParameter<ofColor>*>(param)->get();
                        groupJson[key] = color.getHex();
                    }
                }catch (const std::exception& e) {
                    ofLogError() << "PresetManager::saveParametersToJson:: Error saving value for key " << key << ": " << e.what();
                }
            }
        }
        j[paramGroup->groupName] = groupJson; // Assuming the first key is the group name
    }

    std::ofstream file(jsonFilePath);
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
    }
    else {
        ofLogError() << "PresetManager::saveParametersToJson:: Could not open JSON file for writing";
    }
}


/// <summary>
/// Update the parameters with the interpolation data towards the target values
/// </summary>
/// <param name="parameterGroups"></param>
void PresetManager::updateParameters() {
	if (interpolationDataMap.empty()) {
		return;
	}

    float currentTime = ofGetElapsedTimef();

    for (auto& paramGroup : *params) {
        const std::string& groupName = paramGroup->groupName;

        //auto it = interpolationDataMap.find(paramGroup->parameterMap.begin()->first);
        //if (it != interpolationDataMap.end()) {
            //InterpolationData& interpolationData = it->second;
        if (interpolationDataMap.find(groupName) != interpolationDataMap.end()) {
            InterpolationData& interpolationData = interpolationDataMap[groupName];
            float elapsedTime = currentTime - interpolationData.startTime;
            float t = std::min(elapsedTime / interpolationData.duration, 1.0f);

            for (auto& [key, targetValue] : interpolationData.targetValues) {
                auto param = paramGroup->parameterMap[key];
                if (param == nullptr) {
                    continue;
                }

                const std::string paramTypeName = typeid(*param).name();

                if (paramTypeName == typeid(ofParameter<int>).name()) {
                    int currentValue = dynamic_cast<ofParameter<int>*>(param)->get();
                    if (t > 1.0f) t = 1.0f;
                    int interpolatedValue = ofxeasing::map_clamp(t, 0.0f, 1.0f, currentValue, targetValue, ofxeasing::linear::easeIn);
                    dynamic_cast<ofParameter<int>*>(param)->set(interpolatedValue);
                }
                else if (paramTypeName == typeid(ofParameter<float>).name()) {
                    float currentValue = dynamic_cast<ofParameter<float>*>(param)->get();
                    if (t > 1.0f) t = 1.0f;
                    float interpolatedValue = ofxeasing::map_clamp(t, 0.0f, 1.0f, currentValue, targetValue, ofxeasing::linear::easeIn);
                    dynamic_cast<ofParameter<float>*>(param)->set(interpolatedValue);
                }
            }

            if (currentTime - interpolationData.startTime >= interpolationData.duration) {
                interpolationDataMap.erase(groupName);
            }
        }
    }
}




/// <summary>
/// Clone a preset to another preset
/// </summary>
void PresetManager::clonePresetTo(int from, int to) {
	std::string fromStr = (from < 10 ? "0" : "") + std::to_string(from);
	std::string toStr = (to < 10 ? "0" : "") + std::to_string(to);
	std::string fromJsonFilePath = "data\\presets\\" + fromStr + ".json";
	std::string toJsonFilePath = "data\\presets\\" + toStr + ".json";

	if (fileExist(fromJsonFilePath)) {
		ofLog() << "PresetManager::clonePresetTo:: Cloning preset " << fromStr << " to " << toStr;
		std::ifstream src(fromJsonFilePath, std::ios::binary);
		std::ofstream dst(toJsonFilePath, std::ios::binary);
		dst << src.rdbuf();
		dst.close();
	}
	else {
		ofLog() << "PresetManager::clonePresetTo:: No json file for source preset " << fromStr;
	}
}


/// <summary>
/// Load the given sequence string into the sequence vector
/// </summary>
/// <param name="seqString"></param>
void PresetManager::loadSequence(const std::string& seqString) {
	this->sequenceString = seqString;

    sequence.clear();
    sequence = parseSequence(this->sequenceString);
	sequenceIndex = 0;

    ofLog() << "PresetManager::loadSequence:: Sequence loaded " << ofToString(sequence);
}

void PresetManager::playSequence() {
	playSequence(sequencePresetDuration, sequenceTransitionDuration);
}

void PresetManager::playSequence(float presetDuration, float transitionDuration) {
    ofLogNotice("PresetManager::playSequence") << "Playing sequence index " << sequenceIndex << " the preset " << sequence[sequenceIndex] << " with presetDur " << presetDuration << " and transitionDur " << transitionDuration;
	this->sequencePresetDuration = presetDuration;
    this->sequenceTransitionDuration = transitionDuration;

	if (sequence.size() == 0) {
		ofLog() << "PresetManager::playSequence:: No sequence to play";
		return;
	}

    ofLog() << "PresetManager::playSequence:: Playing preset " << ofToString(sequence[sequenceIndex]);
	applyPreset(sequence[sequenceIndex], sequenceTransitionDuration);
}



void PresetManager::stopSequence() {
    sequence.clear();
	sequenceIndex = 0;
}


// TODO: Use events
void PresetManager::update() {
	updateParameters();

    if (isPlayingSequence()) {
        float currentTime = ofGetElapsedTimef();
        
        if (isTransitioning) {
            if (currentTime - lastUpdateTime >= sequenceTransitionDuration) {
                isTransitioning = false;
                lastUpdateTime = currentTime;
                onTransitionFinished();
            }
        }
        else {
            if (currentTime - lastUpdateTime >= sequencePresetDuration) {
                lastUpdateTime = currentTime;
                playSequence();
                updateSequenceIndex();
                isTransitioning = true;
                onPresetFinished();
            }
        }
    }
}



/// <summary>
/// Move to the next step in the sequence. Restart if the end is reached
/// </summary>
void PresetManager::updateSequenceIndex() {
	sequenceIndex++;
	if (sequenceIndex >= sequence.size()) {
		sequenceIndex = 0;
	}
}


/// <summary>
/// Event to notify that a preset has finished
/// </summary>
void PresetManager::onPresetFinished() {
    ofNotifyEvent(presetFinishedEvent, this);
}


/// <summary>
/// Event to notify that a transition has finished
/// </summary>
void PresetManager::onTransitionFinished() {
    ofNotifyEvent(transitionFinishedEvent, this);
}


/// <summary>
/// Returns the current preset item in the sequence
/// </summary>
/// <returns></returns>
int PresetManager::getCurrentPreset() {
    if (sequence.size() > 0) {
        return sequence[sequenceIndex];
    }
}







/// <summary>
/// Parse an string sequence into a vector of integers
/// </summary>
/// <param name="input">example: 1, 2, 3 - 6, 2</param>
std::vector<int> PresetManager::parseSequence(std::string& input) {
    std::vector<int> s;

    input = removeInvalidCharacters(input);

    std::istringstream stream(input);
    std::string token;

    while (std::getline(stream, token, ',')) {

        // Handle ranges
        if (token.find('-') != std::string::npos) {
            std::vector<int> sr;

            auto rangeParts = splitString(token, '-');

            if (rangeParts.size() == 2) {
                int start = std::stoi(rangeParts[0]);
                int end = std::stoi(rangeParts[1]);

                // Handle reversed ranges
				if (start > end) {
					std::swap(start, end);
				}
                for (int i = start; i <= end; ++i) {
                    sr.push_back(i);
                }
                if (std::stoi(rangeParts[0]) > std::stoi(rangeParts[1])) {
					std::reverse(sr.begin(), sr.end());
                }
            }

            for (auto& i : sr) {
                s.push_back(i);
            }
        }

        // Handle single numbers
        else {
            s.push_back(std::stoi(token));
        }
    }
    return s;
}

/// <summary>
/// Returns a vector of strings from a string separated by a delimiter
/// </summary>
/// <param name="str"></param>
/// <param name="delimiter"></param>
/// <returns></returns>
std::vector<std::string> PresetManager::splitString(const std::string& str, char delimiter) const {
    std::vector<std::string> parts;
    std::istringstream stream(str);
    std::string part;

    while (std::getline(stream, part, delimiter)) {
        parts.push_back(part);
    }
    return parts;
}


/// <summary>
/// Removes invalid characters but digits, comma and dashes
/// </summary>
/// <param name="input"></param>
/// <returns></returns>
std::string PresetManager::removeInvalidCharacters(const std::string& input) {
    std::string result;
    std::copy_if(input.begin(), input.end(), std::back_inserter(result), [](char c) {
        return std::isdigit(c) || c == ',' || c == '-';
        });
    return result;
}


