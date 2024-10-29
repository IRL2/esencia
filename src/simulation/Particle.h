#pragma once
#include "ofMain.h"

// it is better to have the Particle definition in a separated class, so others can use it

// todo: should this be converted to a proper class?

struct Particle {
    // kernel simpler types wip 
    float x, y;
    float vx, vy;
    float radius;

    // todo: use glm::vec2 everywhere
    //glm::vec2 position;
    //glm::vec2 velocity;
    //float kineticEnergy;
    //std::vector<float> minimumDistance;
    //std::vector<float> LJenergyTermA;
    //std::vector<float> LJenergyTermB;
    //std::vector<float> LJgradientTermA;
    //std::vector<float> LJgradientTermB;
    //float mass;
};

