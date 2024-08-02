#pragma once
#include "ofMain.h"

// it is better to have the Particle definition in a separated class, so others can use it

// todo: should this be converted to a proper class?

struct Particle {
    // kernel simpler types wip 
    float x, y;
    float vx, vy;

    // todo: use glm::vec2 everywhere
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


// lighter structure for kernel program, using simpler types // wip 
struct Particle2 {
    float x, y;
    float vx, vy;
};