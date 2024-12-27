#pragma once

#include <fstream>
#include "ofJson.h"
#include "ofLog.h"
#include "parameters/ParametersBase.h"

class PresetManager {

private:
    void saveParametersToJson(const std::string& jsonFilePath, const std::vector<ParametersBase*>& parameterGroups);
	void applyJsonToParameters(const std::string& jsonFilePath, std::vector<ParametersBase*>& parameterGroups);
    bool fileExist(const std::string& jsonFilePath);

public:
    void applyPreset(int id, std::vector<ParametersBase*>& parameterGroups);
    void savePreset(int id, const std::vector<ParametersBase*>& parameterGroups);
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
    }

    // Parse the JSON file
    ofJson j;
    file >> j;

    // Iterate over all items in the JSON
    for (auto& [group, v] : j.items()) {  // first level is the parameter group

        // Iterate over all parameter groups to find the corresponding 1st level set from the json
        for (auto& paramGroup : parameterGroups) {
            if (paramGroup->parameterMap.find(group) != paramGroup->parameterMap.end()) {

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

                    if (paramTypeName == typeid(ofParameter<bool>).name()) {
                        dynamic_cast<ofParameter<bool>*>(param)->set(value.get<bool>());
                    }
                    else if (paramTypeName == typeid(ofParameter<int>).name()) {
                        dynamic_cast<ofParameter<int>*>(param)->set(value.get<int>());
                    }
                    else if (paramTypeName == typeid(ofParameter<float>).name()) {
                        dynamic_cast<ofParameter<float>*>(param)->set(value.get<float>());
                    }
                }
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

                if (paramTypeName == typeid(ofParameter<bool>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<bool>*>(param)->get();
                }
                else if (paramTypeName == typeid(ofParameter<int>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<int>*>(param)->get();
                }
                else if (paramTypeName == typeid(ofParameter<float>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<float>*>(param)->get();
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

void PresetManager::savePreset(int id, const std::vector<ParametersBase*>& parameterGroups) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    saveParametersToJson(jsonFilePath, parameterGroups);
}


