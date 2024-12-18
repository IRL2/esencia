#pragma once
#include "ParametersBase.h"

struct RenderParameters : public ParametersBase {
    ofParameter<int> size = 3;
    ofParameter<ofColor> color;
    ofParameter<glm::vec2> windowSize;
    ofParameter<bool> useShaders = false;
    ofParameter<bool> useFaketrails = false;
    ofParameter<bool> showVideoPreview = false;
    ofParameter<float> fakeTrialsVisibility = 0.0;
    ofParameter<float> videopreviewVisibility = 0.0;
    ofParameter<ofColor> videoColor;

    ofParameterGroup parameters;

    RenderParameters() {
        parameters.setName("Render Parameters");
        parameters.add(size.set("Size", 3));
        parameters.add(color.set("Color", ofColor::white));
        parameters.add(windowSize.set("Window Size", glm::vec2(0.0f, 0.0f)));
        parameters.add(useShaders.set("Use Shaders", false));
        parameters.add(useFaketrails.set("Use Fake Trails", false));
        parameters.add(showVideoPreview.set("Show Video Preview", false));
        parameters.add(fakeTrialsVisibility.set("Fake Trails Visibility", 0.0f));
        parameters.add(videopreviewVisibility.set("Video Preview Visibility", 0.0f));
        parameters.add(videoColor.set("Video Color", ofColor::black));
    }

    ofParameterGroup& getParameters() override {
        return parameters;
    }
};