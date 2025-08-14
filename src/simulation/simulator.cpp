#include "simulator.h"
#include "ofLog.h"
#include <iomanip>
#include <algorithm>

void Simulator::setup(SimulationParameters* params, GuiApp* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    particles.setup(parameters->amount.getMax(), parameters->amount);
    particles.updateRadiuses(parameters->radius);

    setupComputeShader();
    setupCollisionBuffer();

    glGenTextures(1, &depthFieldTexture);
    glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    std::vector<float> initialDepth(width * height, 0.5f);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, initialDepth.data());
    glBindTexture(GL_TEXTURE_2D, 0);

    parameters->amount.addListener(this, &Simulator::onGUIChangeAmmount);
    parameters->radius.addListener(this, &Simulator::onGUIChangeRadius);
    parameters->applyThermostat.addListener(this, &Simulator::onApplyThermostatChanged);
    parameters->targetTemperature.addListener(this, &Simulator::onTemperatureChanged);
    parameters->coupling.addListener(this, &Simulator::onCouplingChanged);
    parameters->depthFieldScale.addListener(this, &Simulator::onDepthFieldScaleChanged);
    parameters->enableCollisionLogging.addListener(this, &Simulator::onCollisionLoggingChanged);


    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);

    // Create and initialize the SSBO
    glGenBuffers(1, &ssboParticles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.active.size() * sizeof(Particle), particles.active.data(), GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::setupCollisionBuffer() {
    // Initialize internal collision buffer for GPU operations
    collisionBuffer.maxCollisions = MAX_COLLISIONS_PER_FRAME;
    collisionBuffer.collisionCount = 0;
    collisionBuffer.frameNumber = 0;
    collisionBuffer.padding = 0;
    collisionBuffer.collisions.resize(MAX_COLLISIONS_PER_FRAME);

    // Initialize public collision data (for external access)
    collisionData.maxCollisions = MAX_COLLISIONS_PER_FRAME;
    collisionData.collisionCount = 0;
    collisionData.frameNumber = 0;
    collisionData.padding = 0;
    collisionData.collisions.resize(MAX_COLLISIONS_PER_FRAME);

    // Create GPU buffer
    glGenBuffers(1, &ssboCollisions);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCollisions);

    // Calculate total buffer size: header (4 uints) + collision array
    size_t headerSize = 4 * sizeof(uint32_t);
    size_t collisionArraySize = MAX_COLLISIONS_PER_FRAME * sizeof(CollisionData);
    size_t totalBufferSize = headerSize + collisionArraySize;

    glBufferData(GL_SHADER_STORAGE_BUFFER, totalBufferSize, nullptr, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCollisions);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ofLogNotice("Simulator::setupCollisionBuffer") << "Collision buffer created with " << MAX_COLLISIONS_PER_FRAME << " max collisions";
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

    // Set uniforms only once
    deltaTimeLocation = glGetUniformLocation(computeShaderProgram, "deltaTime");
    worldSizeLocation = glGetUniformLocation(computeShaderProgram, "worldSize");
    targetTemperatureLocation = glGetUniformLocation(computeShaderProgram, "targetTemperature");
    couplingLocation = glGetUniformLocation(computeShaderProgram, "coupling");
    applyThermostatLocation = glGetUniformLocation(computeShaderProgram, "applyThermostat");
    depthFieldScaleLocation = glGetUniformLocation(computeShaderProgram, "depthFieldScale");
    videoOffsetLocation = glGetUniformLocation(computeShaderProgram, "videoOffset");
    videoScaleLocation = glGetUniformLocation(computeShaderProgram, "videoScale");
    sourceSizeLocation = glGetUniformLocation(computeShaderProgram, "sourceSize");
    ljEpsilonLocation = glGetUniformLocation(computeShaderProgram, "ljEpsilon");
    ljCutoffLocation = glGetUniformLocation(computeShaderProgram, "ljCutoff");
    maxForceLocation = glGetUniformLocation(computeShaderProgram, "maxForce");
    depthFieldLocation = glGetUniformLocation(computeShaderProgram, "depthField");
    enableCollisionLoggingLocation = glGetUniformLocation(computeShaderProgram, "enableCollisionLogging");
}

void Simulator::update() {
    currentFrameNumber++;
    updateParticlesOnGPU();
    if (parameters->enableCollisionLogging) {
        readCollisionData();
        // Copy internal collision data to public collision data for external access
        collisionData = collisionBuffer;
    }
}

void Simulator::updateParticlesOnGPU() {
    // Reset collision counter for this frame
    if (parameters->enableCollisionLogging) {
        collisionBuffer.collisionCount = 0;
        collisionBuffer.frameNumber = currentFrameNumber;

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCollisions);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32_t), &collisionBuffer);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    glUseProgram(computeShaderProgram);

    glUniform1f(deltaTimeLocation, 0.01f);
    glUniform2f(worldSizeLocation, parameters->worldSize->x, parameters->worldSize->y);
    glUniform1f(targetTemperatureLocation, targetTemperature);
    glUniform1f(couplingLocation, coupling);
    glUniform1i(applyThermostatLocation, applyThermostat ? 1 : 0);
    glUniform1f(depthFieldScaleLocation, hasDepthField ? depthFieldScale : 0.0f);
    glUniform2f(videoOffsetLocation, videoRect.x, videoRect.y);
    glUniform2f(videoScaleLocation, videoScaleX, videoScaleY);
    glUniform2f(sourceSizeLocation, sourceWidth, sourceHeight);
    glUniform1f(ljEpsilonLocation, ljEpsilon);
    glUniform1f(ljCutoffLocation, ljCutoff);
    glUniform1f(maxForceLocation, maxForce);
    glUniform1i(enableCollisionLoggingLocation, parameters->enableCollisionLogging ? 1 : 0);

    // Bind depth field texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
    glUniform1i(depthFieldLocation, 0);

    // Bind particle buffer and collision buffer
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssboCollisions);

    glDispatchCompute((particles.active.size() + 511) / 512, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    // Read back particle data
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    Particle* ptr = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(particles.active.data(), ptr, particles.active.size() * sizeof(Particle));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::readCollisionData() {
    // Read back collision data from GPU
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboCollisions);

    // First read the header to get collision count
    uint32_t* headerPtr = (uint32_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 4 * sizeof(uint32_t), GL_MAP_READ_BIT);
    if (headerPtr) {
        collisionBuffer.collisionCount = headerPtr[0];
        collisionBuffer.maxCollisions = headerPtr[1];
        collisionBuffer.frameNumber = headerPtr[2];
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        // If there are collisions, read the collision data
        if (collisionBuffer.collisionCount > 0) {
            uint32_t actualCollisions = std::min(collisionBuffer.collisionCount, static_cast<uint32_t>(MAX_COLLISIONS_PER_FRAME));

            size_t headerSize = 4 * sizeof(uint32_t);
            size_t collisionDataSize = actualCollisions * sizeof(CollisionData);

            CollisionData* collisionPtr = (CollisionData*)glMapBufferRange(
                GL_SHADER_STORAGE_BUFFER,
                headerSize,
                collisionDataSize,
                GL_MAP_READ_BIT
            );

            if (collisionPtr) {
                // Copy collision data
                for (uint32_t i = 0; i < actualCollisions; i++) {
                    collisionBuffer.collisions[i] = collisionPtr[i];
                }
                glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
            }
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::updateVideoRect(const ofRectangle& rect) {
    videoRect = rect;
    videoScaleX = videoRect.width / sourceWidth;
    videoScaleY = videoRect.height / sourceHeight;
}

void Simulator::onGUIChangeAmmount(float& value) {
    particles.resize(value);

    // Update the SSBO with the new size
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.active.size() * sizeof(Particle), particles.active.data(), GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Simulator::onRenderwindowResize(glm::vec2& worldSize) {
    updateWorldSize(worldSize.x, worldSize.y);
    particles.randomizePoolPositions();
}

void Simulator::updateWorldSize(int _width, int _height) {
    width = _width;
    height = _height;
    parameters->worldSize.set(glm::vec2(_width, _height));
    updateVideoRect(ofRectangle(0, 0, _width, _height));
}

void Simulator::updateDepthFieldTexture() {
    if (!hasDepthField) return;

    const unsigned char* pixels = currentDepthField->getPixels().getData();
    int frameWidth = currentDepthField->getWidth();
    int frameHeight = currentDepthField->getHeight();
    std::vector<float> normalizedPixels(frameWidth * frameHeight);

    // Normalize and invert pixels
    for (int i = 0; i < frameWidth * frameHeight; i++) {
        normalizedPixels[i] = 1.0f - (pixels[i] * INV255);
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

void Simulator::recieveFrame(ofxCvGrayscaleImage& frame) {
    if (frame.getWidth() == 0 || frame.getHeight() == 0) return;
    currentDepthField = &frame;
    hasDepthField = true;
    updateDepthFieldTexture();
}

void Simulator::onGUIChangeRadius(int& value) {
    particles.updateRadiuses((int)value);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.active.size() * sizeof(Particle), particles.active.data(), GL_DYNAMIC_COPY);
}

void Simulator::onApplyThermostatChanged(bool& value) {
    applyThermostat = value;
}

void Simulator::onTemperatureChanged(float& value) {
    targetTemperature = value;
}

void Simulator::onCouplingChanged(float& value) {
    coupling = value;
}

void Simulator::onDepthFieldScaleChanged(float& value) {
    depthFieldScale = value;
}

void Simulator::onCollisionLoggingChanged(bool& value) {
    ofLogNotice("Simulator") << "Collision logging " << (value ? "enabled" : "disabled") << " via GUI";
}

void Simulator::applyBerendsenThermostat() {
    // Placeholder for Berendsen thermostat function
}

void Simulator::keyReleased(ofKeyEventArgs& e) {
    int key = e.keycode;
    switch (key) {
    case 'c':
    case 'C':
        parameters->enableCollisionLogging = !parameters->enableCollisionLogging;
        ofLogNotice("Simulator") << "Collision logging " << (parameters->enableCollisionLogging ? "enabled" : "disabled") << " via keyboard";
        break;
    default:
        break;
    }
}
