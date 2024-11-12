#pragma once
#include <vector>
#include <algorithm>
//#include <ranges>
#include "ofMain.h"


struct Particle {
    int index;
    glm::vec2 position;
    glm::vec2 velocity;
    float mass = 5.0;
    float kineticEnergy = 1.0;
    float radius = 2.0;
    std::vector<float> minimumDistance;
    std::vector<float> LJenergyTermA;
    std::vector<float> LJenergyTermB;
    std::vector<float> LJgradientTermA;
    std::vector<float> LJgradientTermB;
};


class ParticleSystem {
private:
    //size_t activeCount;

public:
    std::vector<Particle> active;
    std::vector<Particle> pool;


    // Constructor that takes the maximum pool size
    //ParticleSystem(size_t maxPoolSize) : pool(maxPoolSize), activeCount(0) {
    ParticleSystem(size_t maxPoolSize, size_t initialAmmount) {

        ofLogVerbose("ParticleSystem::ParticleSystem():Initializing particle pool with: ") << maxPoolSize;

        pool.resize(maxPoolSize);

        int i = 0;
        // Initialize the particles in the pool
        for (auto& p : pool) {
            p.index = i++;
            // Set initial position, velocity, etc.
            p.position.x = ofRandomWidth();
            p.position.y = ofRandomHeight();
            p.velocity.x = ofRandom(-2.0, 2.0);
            p.velocity.y = ofRandom(-2.0, 2.0);
        }

        active = pool;
        //activeCount = maxPoolSize;

        resize(initialAmmount);
    }

    // Resize the active particle count
    void resize(size_t newActiveCount) {
        ofLogVerbose("ParticleSystem::resize():Resizing to ") << newActiveCount;

        
        size_t currentSize = active.size();
        size_t newSize = std::min(newActiveCount, pool.size());

        if (currentSize == newSize) return;

        //active = std::vector<Particle>(pool.begin(), pool.begin() + newSize); // working, but does not update pool

        if (newSize < currentSize) { // remove items from active

            // first update the pool with current active values
            for (int i = newSize; i < currentSize; i++) {
                pool[i] = active[i];
            }


            // then, remove elements from active
            active.erase(active.begin() + newSize, active.end());
        }
        else if (newSize > currentSize) {  // add elements to active from pool
            for (int i = currentSize; i < newSize; ++i) {
                active.push_back(pool[i]);
            }
        }

        //activeCount = newSize;
    }

    size_t size() {
        return active.size();
    }
};

