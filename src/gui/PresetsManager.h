#pragma once

#include <fstream>
#include "ofJson.h"
#include "ofLog.h"
#include "parameters/ParametersBase.h"
#include "ofxEasing.h"

struct InterpolationData {
    float startTime;
    float duration;
    std::unordered_map<std::string, float> targetValues;
};


class PresetManager {
private:
    void saveParametersToJson(const std::string& jsonFilePath, const std::vector<ParametersBase*>& parameterGroups);
	void applyJsonToParameters(const std::string& jsonFilePath, std::vector<ParametersBase*>& parameterGroups);
    bool fileExist(const std::string& jsonFilePath);

    std::unordered_map<std::string, InterpolationData> interpolationDataMap;

public:
    void applyPreset(int id, std::vector<ParametersBase*>& parameterGroups);
    void savePreset(int id, const std::vector<ParametersBase*>& parameterGroups);
    void updateParameters(std::vector<ParametersBase*>& parameterGroups);
};



/// <summary>
/// Apply the values from a JSON file to the parameters
/// </summary>
/// <param name="jsonFilePath">Full path to the json file</param>
/// <param name="parameterGroups">A vector with references to the destination parameters</param>
void PresetManager::applyJsonToParameters(const std::string& jsonFilePath, std::vector<ParametersBase*>& parameterGroups) {
    ofLog() << "Applying JSON to esencia parameters from " << jsonFilePath;

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
        for (auto& paramGroup : parameterGroups) {
            if (paramGroup->parameterMap.find(group) != paramGroup->parameterMap.end()) {

                InterpolationData interpolationData;
                interpolationData.startTime = ofGetElapsedTimef();
                interpolationData.duration = 1.0f; // Set the duration for interpolation

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

void PresetManager::applyPreset(int id, std::vector<ParametersBase*>& parameterGroups) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    if (fileExist(jsonFilePath)) {
        applyJsonToParameters(jsonFilePath, parameterGroups);
    }
    else {
        ofLog() << "PresetManager::applyPreset:: No json file for preset " << idStr;
    }
}

void PresetManager::savePreset(int id, const std::vector<ParametersBase*>& parameterGroups) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    saveParametersToJson(jsonFilePath, parameterGroups);
}


bool PresetManager::fileExist(const std::string& jsonFilePath) {
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
        return false;
    }
    return true;
}


void PresetManager::saveParametersToJson(const std::string& jsonFilePath, const std::vector<ParametersBase*>& parameterGroups) {
    ofLog() << "Saving JSON to esencia parameters to " << jsonFilePath;

    ofJson j;

    for (const auto& paramGroup : parameterGroups) {
        ofJson groupJson;
        for (const auto& [key, param] : paramGroup->parameterMap) {
            if (param != nullptr) {
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
                    ofLogError() << "Error saving value for key " << key << ": " << e.what();
                }
            }
        }
        j[paramGroup->parameterMap.begin()->first] = groupJson; // Assuming the first key is the group name
    }

    std::ofstream file(jsonFilePath);
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
    }
    else {
        ofLogError() << "Could not open JSON file for writing";
    }
}



void PresetManager::updateParameters(std::vector<ParametersBase*>& parameterGroups) {
    float currentTime = ofGetElapsedTimef();

    for (auto& paramGroup : parameterGroups) {
        auto it = interpolationDataMap.find(paramGroup->parameterMap.begin()->first);
        if (it != interpolationDataMap.end()) {
            InterpolationData& interpolationData = it->second;

            for (auto& [key, targetValue] : interpolationData.targetValues) {
                auto param = paramGroup->parameterMap[key];
                if (param == nullptr) {
                    continue;
                }

                const std::string paramTypeName = typeid(*param).name();

                if (paramTypeName == typeid(ofParameter<int>).name()) {
                    int currentValue = dynamic_cast<ofParameter<int>*>(param)->get();
                    float t = (currentTime - interpolationData.startTime) / interpolationData.duration;
                    if (t > 1.0f) t = 1.0f;
                    int interpolatedValue = ofxeasing::map(t, 0.0f, 1.0f, currentValue, targetValue, ofxeasing::linear::easeIn);
                    dynamic_cast<ofParameter<int>*>(param)->set(interpolatedValue);
                }
                else if (paramTypeName == typeid(ofParameter<float>).name()) {
                    float currentValue = dynamic_cast<ofParameter<float>*>(param)->get();
                    float t = (currentTime - interpolationData.startTime) / interpolationData.duration;
                    if (t > 1.0f) t = 1.0f;
                    float interpolatedValue = ofxeasing::map(t, 0.0f, 1.0f, currentValue, targetValue, ofxeasing::linear::easeIn);
                    dynamic_cast<ofParameter<float>*>(param)->set(interpolatedValue);
                }
            }

            if (currentTime - interpolationData.startTime >= interpolationData.duration) {
                interpolationDataMap.erase(it);
            }
        }
    }
}
