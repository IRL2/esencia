#pragma once

#include "ofxGuiExtended.h"

class ParametersBase {
public:
    virtual ~ParametersBase() = default;

    // Optionally provide a method to retrieve the parameter group
    virtual ofParameterGroup& getParameters() = 0;
};
