#include "simulator.h"

// <summary>
// assign params pointer and listeners to value changes
// </summary>
// <param name="params">pointer from the gui structure</param>

void Simulator::setup(Gui::SimulationParameters* params, Gui* globalParams) {
    parameters = params;
    globalParameters = globalParams;
    parameters->ammount.addListener(this, &Simulator::onGUIChangeAmmount);
    parameters->radius.addListener(this, &Simulator::onGUIChangeRadius);
    parameters->applyThermostat.addListener(this, &Simulator::onApplyThermostatChanged);
    parameters->targetTemperature.addListener(this, &Simulator::onTemperatureChanged);
    parameters->coupling.addListener(this, &Simulator::onCouplingChanged);

    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);
    //initializeParticles(parameters->ammount);
}


void Simulator::update() {
    for (auto &particle : particles.getActiveParticles()) {
            updateParticle(particle, timeStep);
        }

    
//            initializeParticles(parameters->ammount);

        if(applyThermostat) {
            applyBerendsenThermostat();
        }
        for (auto &particle : particles.getActiveParticles()) {
            checkWallCollisions(particle);
        }
}
void Simulator::calculateEnergyTerms() {
    int count = particles.getActiveCount();
    for (int i = 0; i < count; ++i) {
        Particle &particle = particles.getActiveParticles()[i];
        particle.minimumDistance.resize(count);
        particle.LJenergyTermA.resize(count);
        particle.LJenergyTermB.resize(count);
        particle.LJgradientTermA.resize(count);
        particle.LJgradientTermB.resize(count);

        for (int j = 0; j < count; ++j) {
            float radiusi = particle.radius;
            float radiusj = particles.getActiveParticles()[j].radius;

            float MinDistance = 2.0f * (radiusi + radiusj);
            particle.minimumDistance[j] = MinDistance * MinDistance;
            particle.LJenergyTermA[j] = epsilon * pow(MinDistance, 12.0f);
            particle.LJenergyTermB[j] = -2.0f * epsilon * pow(MinDistance, 6.0f);
            particle.LJgradientTermA[j] = -12.0f * particle.LJenergyTermA[j];
            particle.LJgradientTermB[j] = -6.0f * particle.LJenergyTermB[j];
        }
    }
}

glm::vec2 Simulator::computeForce(Particle &particle) {
    glm::vec2 totalForce(0.0f, 0.0f);
    float maxForce = 100000.0f;
    float minForce = 1e-6f;

    int i = &particle - &particles.getActiveParticles()[0]; // Get the index of the particle

    for (int j = 0; j < particles.getActiveCount(); ++j) {
        if (i == j) continue;

        Particle &other = particles.getActiveParticles()[j];
        glm::vec2 rVec = particle.position - other.position;
        float r = glm::length(rVec);
        if (r < epsilon) {
            r = epsilon; // Prevent division by zero by adding epsilon
        }
        if (i < particle.minimumDistance.size() && j < particle.minimumDistance.size()) {
            float cutoffDistance = particle.minimumDistance[j];
            if (r * r < cutoffDistance && r > 0) {
                float LJgradientTermA = particle.LJgradientTermA[j];
                float LJgradientTermB = particle.LJgradientTermB[j];

//                r = sqrt(r * r); // Adjust separation distance

                float forceX = (other.position.x - particle.position.x) *
                               (LJgradientTermA / pow(r, 13.0f) + LJgradientTermB / pow(r, 7.0f)) / r;
                float forceY = (other.position.y - particle.position.y) *
                               (LJgradientTermA / pow(r, 13.0f) + LJgradientTermB / pow(r, 7.0f)) / r;

                glm::vec2 force(forceX, forceY);

                if (glm::length(force) > maxForce) force = glm::normalize(force) * maxForce;
                else if (glm::length(force) < minForce) force = glm::vec2(0.0f, 0.0f);

                totalForce += force;
            }
        }
    }
    return totalForce;
}

void Simulator::updateParticle(Particle &particle, float deltaTime) {
    // Velocity Verlet Integration
    glm::vec2 force = computeForce(particle);
    glm::vec2 acceleration = force / particle.mass;
    glm::vec2 newVelocity = particle.velocity + deltaTime * acceleration;
    glm::vec2 newPosition = particle.position + deltaTime * newVelocity;

    particle.velocity = newVelocity;
    particle.position = newPosition;
}

void Simulator::applyBerendsenThermostat() {
    float targetTemp = targetTemperature; // m_EquilibriumTemperature
    float tau = coupling; // m_BerendsenThermostatCoupling
    float kB = 8.314; // Boltzmann constant

    // temp calculation
    float currentTemperature = 0.0;
    float kineticEnergy = 0.0;
    for (const auto& particle : particles.getActiveParticles()) {
        kineticEnergy += 0.5f * particle.mass * glm::length2(particle.velocity);
    }
    currentTemperature = kineticEnergy / (3.0 * kB);
    

    float scaleFactor = sqrt(targetTemp / (tau * currentTemperature));

    scaleFactor = ofClamp(scaleFactor, 0.95, 1.05);
    for (auto& particle : particles.getActiveParticles()) {
        particle.velocity *= scaleFactor;
    }
}

void Simulator::checkWallCollisions(Particle &particle) {
    if (particle.position.x < 0 || particle.position.x > width) {
        particle.velocity.x *= -1.0f;
        particle.position.x = ofClamp(particle.position.x, 0, width);
    }
    if (particle.position.y < 0 || particle.position.y > height) {
        particle.velocity.y *= -1.0f;
        particle.position.y = ofClamp(particle.position.y, 0, height);
    }
}

void Simulator::initializeParticles(float ammount) {
    initializeParticles((int)ammount);
}

void Simulator::initializeParticles(int ammount) {
    particles.resize(ammount); // Resize to hold the new particles

    calculateEnergyTerms(); // Calculate the energy terms after initializing particles
}

#pragma region Listeners

void Simulator::onGUIChangeAmmount(float& value) {
    initializeParticles(value);
}

void Simulator::onGUIChangeRadius(int& value) {
    for (auto &p : particles.getAllParticles()) {
        p.radius = value;
    }
}

void Simulator::onRenderwindowResize(glm::vec2& worldSize) {
    updateWorldSize(worldSize.x, worldSize.y);
}

void Simulator::updateWorldSize(int _width, int _height) {
    ofLogNotice("Simulator::updateWorldSize()") << "wew size: " << _width << "," << _height;

    width = _width;
    height = _height;
    parameters->worldSize.set(glm::vec2(_width, _height)); // we may also use the parameters->worlSize.x directly
}

void Simulator::recieveFrame(ofxCvGrayscaleImage frame) {
    return;
}

void Simulator::onApplyThermostatChanged(bool &value) {
    applyThermostat = value;
    if (applyThermostat) {
        std::printf("Thermostat enabled\n");
    } else {
        std::printf("Thermostat disabled\n");
    }
}

void Simulator::onTemperatureChanged(float &value) {
    targetTemperature = value;
//    std::printf("%f\n",targetTemperature);
}

void Simulator::onCouplingChanged(float &value) {
    coupling = value;
//    std::printf("%f\n", coupling);
}
#pragma endregion
