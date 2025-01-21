#include "simulator.h"
#include "ofLog.h"

void Simulator::setup(SimulationParameters* params, GuiApp* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    particles.setup(parameters->amount.getMax(), parameters->amount);
    particles.updateRadiuses(parameters->radius);

    setupComputeShader();

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
 

    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);

    // Create and initialize the SSBO
    glGenBuffers(1, &ssboParticles);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    glBufferData(GL_SHADER_STORAGE_BUFFER, particles.active.size() * sizeof(Particle), particles.active.data(), GL_DYNAMIC_COPY);
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
}

void Simulator::update() {
    updateParticlesOnGPU();
}


void Simulator::updateParticlesOnGPU() {
    glUseProgram(computeShaderProgram);

    glUniform1f(deltaTimeLocation, 0.01f);
    glUniform2f(worldSizeLocation, width, height);
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

    // Bind depth field texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthFieldTexture);
    glUniform1i(depthFieldLocation, 0);

    // Bind particle buffer and dispatch compute shader
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssboParticles);
    glDispatchCompute((particles.active.size() + 255) / 256, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssboParticles);
    Particle* ptr = (Particle*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    memcpy(particles.active.data(), ptr, particles.active.size() * sizeof(Particle));
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
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
    ofLogNotice("Simulator::updateWorldSize()") << "wew size: " << _width << "," << _height;

    width = _width;
    height = _height;
    parameters->worldSize.set(glm::vec2(_width, _height)); // we may also use the parameters->worlSize.x directly

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

void Simulator::recieveFrame(ofxCvGrayscaleImage & frame) {
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

void Simulator::applyBerendsenThermostat() {
    // Placeholder for Berendsen thermostat function
}

//std::vector<Particle>& Simulator::getParticles() {
//    return particles;
//}

void Simulator::keyReleased(ofKeyEventArgs& e) {
    //
}
