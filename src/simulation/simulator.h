#pragma once

#include "ofMain.h"
#include "../gui/Gui.h"
#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here
#include "Particle.h"

class Simulator
{
public:
	// lifecycle
	void setup(Gui::SimulationParameters* params, Gui* globalParams);
	void update();

	// listeners
	void updateWorldSize(int _width, int _height);
    void onRenderwindowResize(glm::vec2& worldSize);
	void onGUIChangeAmmount(float& value);
    void onGUIChangeRadius(int& value);
    void onApplyThermostatChanged(bool& value);
    void onTemperatureChanged(float& value);
    void onCouplingChanged(float& value);
    void applyBerendsenThermostat();

	void recieveFrame(ofxCvGrayscaleImage frame);

	// properties
	vector<Particle> particles;
    float epsilon = 5.0f;
    float timeStep = 0.01;
    void calculateEnergyTerms();
    glm::vec2 computeForce(Particle &particle);
    void calculateTotalKineticEnergy(Particle &particle);
    void updateParticle(Particle &particle, float deltaTime);
    void checkWallCollisions(Particle &particle);
    

//    void initializeParticles(int ammount);
	// environment
    int width = 700;
    int height = 600;

	// parameters
	Gui::SimulationParameters* parameters;
	Gui* globalParameters;
    bool applyThermostat = true;
    float targetTemperature = 25000.0;
    float coupling = 0.5;

private:
	void initializeParticles(int ammount);
	void initializeParticles(float ammount);
    
    
};


