#include "particles.h"

/// <summary>
/// Initialize the particle pool and set the initial active set
/// </summary>
/// <param name="maxPoolSize">Maximum number of particles to initialize in the pool</param>
/// <param name="initialAmount">The available active group</param>
void ParticleSystem::setup(size_t maxPoolSize, size_t initialAmount) {
    ofLogVerbose("ParticleSystem::ParticleSystem():Initializing particle pool with: ") << maxPoolSize;

	this->maxPoolSize = maxPoolSize;

    pool.resize(maxPoolSize);

    int i = 0;
    // Initialize the particles in the pool
    for (auto& p : pool) {
        //p.index = i++;
        // Set initial position, velocity, etc.
        p.position.x = ofRandom(1, ofGetWidth()-2);
        p.position.y = ofRandom(1, ofGetHeight()-2);
        p.velocity = glm::vec2(ofRandom(-10.0f, 10.0f), ofRandom(-10.0f, 10.0f));
        p.mass = 5.0;
        p.radius = 2.0;
    }

    active = pool;

    resize(initialAmount);
}

/// <summary>
/// Resize the active particle set
/// </summary>
/// <param name="newActiveAmount">amount of particles in the active set</param>
void ParticleSystem::resize(size_t newActiveAmount) {
    ofLog(OF_LOG_NOTICE) << "ParticleSystem::resize():Resizing to " << newActiveAmount;

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

        // then, remove the excedent from active
        active.erase(active.begin() + newSize, active.end());
    }
    else if (newSize > currentSize) {  // add elements to active from pool
        for (int i = currentSize; i < newSize; ++i) {
            //pool[i].radius = pool[1].radius;
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


/// <summary>
/// Update the radius in all particles, active and pool
/// TODO: If radius is allways the same for all particles, we must use a single variable (maybe on the particle system) instead of having it on each particle, mostly because radius is a visual property not used in the simulation (not as mass for example)
/// </summary>
/// <param name="newRadiuses"></param>
void ParticleSystem::updateRadiuses(float newRadiuses) {
    for (auto &p : active) {
        p.radius = newRadiuses;
    }
    for (auto &p : pool) {
        p.radius = newRadiuses;
    }
}

/// <summary>
// Randomize the positions for the pool particles when screen is resized
// so, they are ready to fill the new screen size, not always at their initial positions
/// </summary>
void ParticleSystem::randomizePoolPositions() {
	for (auto& p : pool) {
		p.position.x = ofRandom(1, ofGetWidth() - 2);
		p.position.y = ofRandom(1, ofGetHeight() - 2);
	}
}
