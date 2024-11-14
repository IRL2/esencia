#include "simulator.h"
#include "ofLog.h"

void Simulator::setup(Gui::SimulationParameters* params, Gui* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    initializeParticles(parameters->ammount);
    setupComputeShader();

    // Add listeners for real-time parameter updates
    parameters->ammount.addListener(this, &Simulator::onGUIChangeAmmount);
    parameters->radius.addListener(this, &Simulator::onGUIChangeRadius);
    parameters->applyThermostat.addListener(this, &Simulator::onApplyThermostatChanged);
    parameters->targetTemperature.addListener(this, &Simulator::onTemperatureChanged);
    parameters->coupling.addListener(this, &Simulator::onCouplingChanged);
    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);
}

void Simulator::initializeParticles(int amount) {
    particles.resize(amount);
    for (auto& particle : particles) {
        particle.position = glm::vec2(ofRandom(1, width), ofRandom(1, height));
        particle.velocity = glm::vec2(ofRandom(-100.0f, 100.0f), ofRandom(-100.0f, 100.0f));
        particle.mass = 5.0f;
        particle.radius = parameters->radius;
    }

    glGenBuffers(1, &ssboParticles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::setupComputeShader() {
    computeShaderProgram = glCreateProgram();
    GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    std::string shaderCode = ofBufferFromFile("shaders/particlesComputeShader.glsl").getText();
    const char* shaderSource = shaderCode.c_str();
    glShaderSource(computeShader, 1, &shaderSource, nullptr);
    glCompileShader(computeShader);

    GLint compileStatus;
    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(computeShader, 512, nullptr, buffer);
    }

    glAttachShader(computeShaderProgram, computeShader);
    glLinkProgram(computeShaderProgram);

    GLint linkStatus;
    glGetProgramiv(computeShaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(computeShaderProgram, 512, nullptr, buffer);
        ofLogError() << "Program linking failed: " << buffer;
    }

    glDeleteShader(computeShader);
}

void Simulator::update() {
    updateParticlesOnGPU();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    Particle* ptr = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(particles.data(), ptr, particles.size() * sizeof(Particle));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::updateParticlesOnGPU() {
    glUseProgram(computeShaderProgram);

    glUniform1f(glGetUniformLocation(computeShaderProgram, "deltaTime"), 0.01f);
    glUniform2f(glGetUniformLocation(computeShaderProgram, "worldSize"), width, height);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "targetTemperature"), targetTemperature);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "coupling"), coupling);
    glUniform1i(glGetUniformLocation(computeShaderProgram, "applyThermostat"), applyThermostat ? 1 : 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
    glDispatchCompute((particles.size() + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(0);
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

void Simulator::onGUIChangeAmmount(float& value) {
    initializeParticles(value);
}

void Simulator::onGUIChangeRadius(int& value) {
    for (auto& particle : particles) {
        particle.radius = value;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.size() * sizeof(Particle), particles.data(), GL_DYNAMIC_COPY);
}

void Simulator::onApplyThermostatChanged(bool& value) {
    applyThermostat = value;
}

void Simulator::onTemperatureChanged(float& value) {
    targetTemperature = value;
    printf("%f\n", targetTemperature);
}

void Simulator::onCouplingChanged(float& value) {
    coupling = value;
    printf("%f\n", coupling);
}

void Simulator::applyBerendsenThermostat() {
    // Placeholder for Berendsen thermostat function
}

std::vector<Particle>& Simulator::getParticles() {
    return particles;
}
