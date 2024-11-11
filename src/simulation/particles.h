#pragma once
#include <vector>
#include <string_view>
#include "ofMain.h"


struct Particle {
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

class ParticlePool {
private:
    std::vector<Particle> particles;
    size_t size;

public:
    class ParticlesView {
    private:
        Particle* data;
        size_t count;

    public:
        ParticlesView(Particle* ptr, size_t size) : data(ptr), count(size) {}

        Particle* begin() { return data; }
        Particle* end() { return data + count; }
        const Particle* begin() const { return data; }
        const Particle* end() const { return data + count; }
        size_t size() const   { return count; } // internally named count so dont get confused with the pool's size
        size_t length() const { return size(); } // internally named count so dont get confused with the pool's size
        Particle& operator[](size_t index) { return data[index]; }
        const Particle& operator[](size_t index) const { return data[index]; }
    };

    ParticlePool(size_t poolSize) : size(0) {
        ParticlePool(poolSize, ofGetWindowWidth(), ofGetWindowHeight());
    }

    ParticlePool(size_t poolSize, float width, float height) : size(0) {
        particles.resize(poolSize);

        for (int i = 0; i <= poolSize; i++) {
            Particle p = particles[i];
            bool isOverlapping = false;
            do {
                isOverlapping = false;
                p.position.x = ofRandom(width);
                p.position.y = ofRandom(height);
                p.velocity.x = ofRandom(-2.0, 2.0);
                p.velocity.y = ofRandom(-2.0, 2.0);

                // Check for overlap with existing particles
                for (const auto& other : particles) {
                    if (abs(p.position.x - other.position.x) < p.radius && abs(p.position.y - other.position.y) < p.radius) {
                        isOverlapping = true;
                        break;
                    }
                }
            } while (isOverlapping); // Try new positions for the current particle until no overlap

            particles[i] = p;
        }
        ofLogVerbose("ParticlesPool::ParticlesPool():Initialized ") << poolSize << " particles";
    }

    std::vector<Particle> getAllParticles() {
        return particles;
    }

    ParticlesView getActiveParticles() {
        return ParticlesView(particles.data(), size);
    }

    void resize(size_t newSize) {
        size = newSize;
    }

    size_t getActiveCount() const { return size; }

    size_t getPoolSize() const { return particles.size(); }

};
