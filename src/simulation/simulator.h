#pragma once

#include "ofMain.h"
#include "../gui/Gui.h"
#include <GL/glew.h>

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    float radius;
    float mass;
};

class Simulator {
public:
    void setup(Gui::SimulationParameters* params, Gui* globalParams);
    void update();
    void updateWorldSize(int _width, int _height);
    void recieveFrame(ofxCvGrayscaleImage frame);
    std::vector<Particle>& getParticles();

private:
    void initializeParticles(int amount);
    void setupComputeShader();
    void updateParticlesOnGPU();
    void updateDepthFieldTexture();

    // Original listeners
    void onRenderwindowResize(glm::vec2& worldSize);
    void onGUIChangeAmmount(float& value);
    void onGUIChangeRadius(int& value);
    void onApplyThermostatChanged(bool& value);
    void onTemperatureChanged(float& value);
    void onCouplingChanged(float& value);
    void applyBerendsenThermostat();

    GLuint ssboParticles;
    GLuint computeShaderProgram;
    GLuint depthFieldTexture;
    std::vector<Particle> particles;

    int width = 640;
    int height = 576;
    bool applyThermostat = true;
    float targetTemperature = 2000.0;
    float coupling = 0.5;
    float depthFieldScale = -500000.0f;
    bool hasDepthField = false;

    ofxCvGrayscaleImage currentDepthField;
    Gui::SimulationParameters* parameters;
    Gui* globalParameters;
};