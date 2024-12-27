#pragma once

#include "ofxGuiExtended.h"


class ParametersBase {
    
public:
    std::unordered_map<std::string, ofAbstractParameter*> parameterMap;

    virtual ~ParametersBase() = default;

	virtual void initializeParameterMap() = 0;
};
