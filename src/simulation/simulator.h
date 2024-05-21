#pragma once

#include "ofMain.h"
#include "../gui/Gui.h"
#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
    float mass = 5.0;
    float kineticEnergy;
    float radius;
    std::vector<float> minimumDistance;
    std::vector<float> LJenergyTermA;
    std::vector<float> LJenergyTermB;
    std::vector<float> LJgradientTermA;
    std::vector<float> LJgradientTermB;
};

class Simulator
{
public:
	// lifecycle
	void setup(Gui::SimulationParameters* params);
	void update();

	// listeners
	void updateWorldSize(int _width, int _height);
	void onGUIChangeAmmount(int& value);
    void onApplyThermostatChanged(bool& value);
    void onTemperatureChanged(float& value);
    void onCouplingChanged(float& value);
    void applyBerendsenThermostat();

	void recieveFrame(ofxCvGrayscaleImage frame);

	// properties
	vector<Particle> particles;
    float epsilon = 10.0f;
    float timeStep = 0.01;
    void calculateEnergyTerms();
    glm::vec2 computeForce(Particle &particle);
    void calculateTotalKineticEnergy(Particle &particle);
    void updateParticle(Particle &particle, float deltaTime);
    void checkWallCollisions(Particle &particle);
    

//    void initializeParticles(int ammount);
	// environment
	int width;
	int height;

	// parameters
	Gui::SimulationParameters* parameters;
	Gui* globalParameters;
    bool applyThermostat = true;
    float targetTemperature = 25000.0;
    float coupling = 0.5;

private:
	void initializeParticles(int ammount);
    

};


