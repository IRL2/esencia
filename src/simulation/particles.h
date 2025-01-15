#pragma once
#include <vector>
#include <algorithm>
//#include <ranges>
#include "ofMain.h"

/// <summary>
/// Partice structure to hold all data from a single particle
/// </summary>
struct Particle {
    //int index;

    glm::vec2 position;
    glm::vec2 velocity;

    float radius = 2.0;
    float mass = 5.0;
    //float kineticEnergy = 1.0;

    //std::vector<float> minimumDistance;
    //std::vector<float> LJenergyTermA;
    //std::vector<float> LJenergyTermB;
    //std::vector<float> LJgradientTermA;
    //std::vector<float> LJgradientTermB;
};


/// <summary>
/// Particle system responsible of provide new particles to the simulation
/// Creates a pool from the begining from which takes in out particles
/// Pool is in sync from the active particle updates
/// </summary>
class ParticleSystem {
public:
	size_t maxPoolSize;

    /// <summary>
    /// The active particle set, the one used by the simulation
    /// </summary>
    std::vector<Particle> active;
    
    /// <summary>
    /// Holds the larger pool set from where new particles comes from
    /// </summary>
    std::vector<Particle> pool;

    void setup(size_t maxPoolSize, size_t initialAmount);

    void resize(size_t newActiveCount);

    size_t size();

    void updateRadiuses(float newRadius);

	void randomizePoolPositions();
};
