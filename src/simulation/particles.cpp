#include "particles.h"

/// <summary>
/// Initialize the particle pool and set the initial active set
/// </summary>
/// <param name="maxPoolSize">Maximum number of particles to initialize in the pool</param>
/// <param name="initialAmount">The available active group</param>
void ParticleSystem::setup(size_t maxPoolSize, size_t initialAmount) {
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

        //TODO: Check for 'collisions at birth'
    }

    active = pool;

    resize(initialAmount);
}

/// <summary>
/// Resize the active particle set
/// </summary>
/// <param name="newActiveAmount">amount of particles in the active set</param>
void ParticleSystem::resize(size_t newActiveAmount) {
    ofLogVerbose("ParticleSystem::resize():Resizing to ") << newActiveAmount;

    size_t currentSize = active.size();
    size_t newSize = std::min(newActiveAmount, pool.size());

    if (currentSize == newSize) return;

    //active = std::vector<Particle>(pool.begin(), pool.begin() + newSize); // basic vector copy, works but does not update pool back with modified active info

    // COMMENT: I wanted to use vector views, because it is more performant: active would be a limited size reference to the original pool vector, so needs zero processing, but I got compilation errors, maybe due the of compiling flags 

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
}

/// <summary>
/// Get the size of the active set
/// </summary>
/// <returns></returns>
size_t ParticleSystem::size() {
    return active.size();
}

