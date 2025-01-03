#pragma once

#include "ofxGuiExtended.h"


class ParametersBase {
    
public:

	// this map holds a list of all parameters and their names
	// its gonna be used to work with the presets
    std::unordered_map<std::string, ofAbstractParameter*> parameterMap;

    virtual ~ParametersBase() = default;

	virtual void initializeParameterMap() = 0;

	std::string groupName;
};
