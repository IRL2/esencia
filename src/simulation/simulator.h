#pragma once

#include "ofMain.h"
#include <GL/glew.h>
#include "../gui/GuiApp.h"
//#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here
#include "particles.h"

struct CollisionData {
    uint32_t particleA;
    uint32_t particleB;
    glm::vec2 positionA;
    glm::vec2 positionB;
    float distance;
    float velocityMagnitude;
    uint32_t valid;
    uint32_t padding; // for alignment
};

struct CollisionBuffer {
    uint32_t collisionCount;
    uint32_t maxCollisions;
    uint32_t frameNumber;
    uint32_t padding;
    std::vector<CollisionData> collisions;
};

class Simulator {
public:
    void setup(SimulationParameters* params, GuiApp* globalParams);
    void update();
    void updateWorldSize(int _width, int _height);
    void recieveFrame(ofxCvGrayscaleImage& frame);
    void updateVideoRect(const ofRectangle& rect);

    void keyReleased(ofKeyEventArgs& e);

    ParticleSystem particles;

    GLint deltaTimeLocation;
    GLint worldSizeLocation;
    GLint targetTemperatureLocation;
    GLint couplingLocation;
    GLint applyThermostatLocation;
    GLint depthFieldScaleLocation;
    GLint videoOffsetLocation;
    GLint videoScaleLocation;
    GLint sourceSizeLocation;
    GLint ljEpsilonLocation;
    GLint ljSigmaLocation;
    GLint ljCutoffLocation;
    GLint maxForceLocation;
    GLint depthFieldLocation;
    GLint enableCollisionLoggingLocation;

    static const size_t MAX_COLLISIONS_PER_FRAME = 1000;

private:
    void setupComputeShader();
    void updateParticlesOnGPU();
    void updateDepthFieldTexture();
    void setupCollisionBuffer();
    void readCollisionData();
    void logCollisions();

    // listeners
    void onRenderwindowResize(glm::vec2& worldSize);
    void onGUIChangeAmmount(float& value);
    void onGUIChangeRadius(int& value);
    void onApplyThermostatChanged(bool& value);
    void onTemperatureChanged(float& value);
    void onCouplingChanged(float& value);
    void onDepthFieldScaleChanged(float& value);
    void onCollisionLoggingChanged(bool& value);
    void applyBerendsenThermostat();

    GLuint ssboParticles;
    GLuint ssboCollisions;
    GLuint computeShaderProgram;
    GLuint depthFieldTexture;

    bool applyThermostat = true;
    float targetTemperature = 2000.0;
    float coupling = 0.5;
    float depthFieldScale = -100000.0f;
    bool hasDepthField = false;
    float ljEpsilon = 10.0f;    // Lennard-Jones well depth
    float ljCutoff = 150.0f;    // Interaction cutoff
    float maxForce = 10000.0f;  // Force clamping

    ofxCvGrayscaleImage* currentDepthField;

    ofRectangle videoRect = ofRectangle(0, 0, -45, -45);
    float videoScaleX = 1.4;
    float videoScaleY = 1.4f;
    int sourceWidth = 640;  // camera resolution width
    int sourceHeight = 576; // camera resolution height

    int width = 640;
    int height = 576;

    SimulationParameters* parameters;
    GuiApp* globalParameters;

    const float INV255 = 1.0f / 255.0f;

    CollisionBuffer collisionBuffer;
    uint32_t currentFrameNumber = 0;
};