#pragma once

#include "ofMain.h"
#include <GL/glew.h>
#include "../gui/GuiApp.h"
//#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here
#include "particles.h"


class Simulator {
public:
	void setup(SimulationParameters* params, GuiApp* globalParams);
    void update();
    void updateWorldSize(int _width, int _height);
    void recieveFrame(ofxCvGrayscaleImage frame);
    void updateVideoRect(const ofRectangle& rect);

	void keyReleased(ofKeyEventArgs& e);

    ParticleSystem particles;


private:
    void setupComputeShader();
    void updateParticlesOnGPU();
    void updateDepthFieldTexture();


	// listeners
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

    bool applyThermostat = true;
    float targetTemperature = 2000.0;
    float coupling = 0.5;
    float depthFieldScale = -100000.0f;
    bool hasDepthField = false;
    float ljEpsilon = 10.0f;    // Lennard-Jones well depth
    float ljSigma = 0.5f;      // Lennard-Jones distance scale
    float ljCutoff = 150.0f;    // Interaction cutoff
    float maxForce = 10000.0f;  // Force clamping

    ofxCvGrayscaleImage currentDepthField;

    ofRectangle videoRect = ofRectangle(0,0,-45,-45);
    float videoScaleX = 1.4;
    float videoScaleY = 1.4f;
    int sourceWidth = 640;  // camera resolution width
    int sourceHeight = 576; // camera resolution height

    int width = 640;
    int height = 576;

    SimulationParameters* parameters;
    GuiApp* globalParameters;
};