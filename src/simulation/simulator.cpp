#include "simulator.h"
#include "ofLog.h"

void Simulator::setup(Gui::SimulationParameters* params, Gui* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    initializeParticles(parameters->ammount);
    setupComputeShader();

    glGenTextures(1, &depthFieldTexture);
    glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    depthFieldScale = 500000.0f;
    std::vector<float> initialDepth(width * height, 0.5f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, initialDepth.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    parameters->ammount.addListener(this, &Simulator::onGUIChangeAmmount);
    parameters->radius.addListener(this, &Simulator::onGUIChangeRadius);
    parameters->applyThermostat.addListener(this, &Simulator::onApplyThermostatChanged);
    parameters->targetTemperature.addListener(this, &Simulator::onTemperatureChanged);
    parameters->coupling.addListener(this, &Simulator::onCouplingChanged);
    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);
}

// In simulator.cpp, replace the current initializeParticles method:

void Simulator::initializeParticles(int amount) {
    particles.resize(amount);

    // Calculate max attempts to prevent infinite loops
    const int maxAttempts = 100;
    int successfulPlacements = 0;

    // Get current window dimensions and bounds
    float minX, maxX, minY, maxY;

    if (videoRect.width <= 0 || videoRect.height <= 0) {
        float scaledWidth = ofGetHeight() * sourceWidth / sourceHeight;
        float xOffset = (scaledWidth - ofGetWidth()) / -2;

        minX = xOffset;
        maxX = xOffset + scaledWidth;
        minY = 0;
        maxY = ofGetHeight();
    }
    else {
        minX = videoRect.x;
        maxX = videoRect.x + videoRect.width;
        minY = videoRect.y;
        maxY = videoRect.y + videoRect.height;
    }

    // First pass: Initialize particles with attempted overlap prevention
    for (int i = 0; i < amount; i++) {
        Particle& particle = particles[i];
        particle.radius = parameters->radius;
        particle.mass = 5.0f;

        bool validPosition = false;
        int attempts = 0;

        while (!validPosition && attempts < maxAttempts) {
            // Generate random position
            particle.position = glm::vec2(
                ofRandom(minX, maxX),
                ofRandom(minY, maxY)
            );

            // Check overlap with previously placed particles
            validPosition = true;
            for (int j = 0; j < i; j++) {
                float minDist = particle.radius + particles[j].radius;
                if (glm::distance(particle.position, particles[j].position) < minDist) {
                    validPosition = false;
                    break;
                }
            }
            attempts++;
        }

        // Initialize velocity regardless of position success
        particle.velocity = glm::vec2(ofRandom(-100.0f, 100.0f), ofRandom(-100.0f, 100.0f));

        if (validPosition) {
            successfulPlacements++;
        }
    }

    // Log placement statistics
    ofLogNotice("Simulator::initializeParticles")
        << "Placed " << successfulPlacements
        << " particles out of " << amount
        << " requested without overlap";

    // Create and initialize the SSBO
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
        ofLogError() << "Shader compilation failed: " << buffer;
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

    // Basic uniforms
    glUniform1f(glGetUniformLocation(computeShaderProgram, "deltaTime"), 0.01f);
    glUniform2f(glGetUniformLocation(computeShaderProgram, "worldSize"), width, height);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "targetTemperature"), targetTemperature);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "coupling"), coupling);
    glUniform1i(glGetUniformLocation(computeShaderProgram, "applyThermostat"), applyThermostat ? 1 : 0);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "depthFieldScale"), hasDepthField ? depthFieldScale : 0.0f);

    // Video scaling uniforms
    glUniform2f(glGetUniformLocation(computeShaderProgram, "videoOffset"), videoRect.x, videoRect.y);
    glUniform2f(glGetUniformLocation(computeShaderProgram, "videoScale"), videoScaleX, videoScaleY);
    glUniform2f(glGetUniformLocation(computeShaderProgram, "sourceSize"), sourceWidth, sourceHeight);

    // Bind depth field texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
    glUniform1i(glGetUniformLocation(computeShaderProgram, "depthField"), 0);

    // Bind particle buffer and dispatch compute shader
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



void Simulator::updateDepthFieldTexture() {
    if (!hasDepthField) return;

    const unsigned char* pixels = currentDepthField.getPixels().getData();
    int frameWidth = currentDepthField.getWidth();
    int frameHeight = currentDepthField.getHeight();
    std::vector<float> normalizedPixels(frameWidth * frameHeight);

    // Normalize and invert pixels
    for (int i = 0; i < frameWidth * frameHeight; i++) {
        normalizedPixels[i] = 1.0f - (pixels[i] / 255.0f);
    }

    // Update texture dimensions if changed
    if (frameWidth != width || frameHeight != height) {
        width = frameWidth;
        height = frameHeight;
        glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, normalizedPixels.data());
    }
    else {
        glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RED, GL_FLOAT, normalizedPixels.data());
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Simulator::recieveFrame(ofxCvGrayscaleImage frame) {
    if (frame.getWidth() == 0 || frame.getHeight() == 0) return;
    currentDepthField = frame;
    hasDepthField = true;
    updateDepthFieldTexture();
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
