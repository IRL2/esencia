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
    void updateVideoRect(const ofRectangle& rect) {
        videoRect = rect;
        videoScaleX = videoRect.width / sourceWidth;
        videoScaleY = videoRect.height / sourceHeight;
    }


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
    void onSigmaChanged(float& value);
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
    float depthFieldScale = 500000.0f;
    bool hasDepthField = false;
    const int MAX_PLACEMENT_ATTEMPTS = 100;
    const float INITIAL_VELOCITY_RANGE = 100.0f;  // +/- range for random velocity
    const float DEFAULT_MASS = 5.0f;
    float ljEpsilon = 5.0f;    // Lennard-Jones well depth
    float ljSigma = 0.5f;      // Lennard-Jones distance scale
    float ljCutoff = 30.0f;    // Interaction cutoff
    float maxForce = 10000.0f;  // Force clamping

    ofxCvGrayscaleImage currentDepthField;
    Gui::SimulationParameters* parameters;
    Gui* globalParameters;

    ofRectangle videoRect;
    float videoScaleX = 1.0f;
    float videoScaleY = 1.0f;
    int sourceWidth = 640;  // camera resolution width
    int sourceHeight = 576; // camera resolution height
};