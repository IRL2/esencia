#include "simulator.h"
#include "ofLog.h"
#include <iomanip>
#include <algorithm>
#include <numeric>

void Simulator::setup(SimulationParameters* params, GuiApp* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    particles.setup(parameters->amount.getMax(), parameters->amount);
    particles.updateRadiuses(parameters->radius);

    setupComputeShader();
    setupCollisionBuffer();
    setupClusterAnalysis();
    setupVACAnalysis();

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

void Simulator::setupClusterAnalysis() {
    clusterData.clusterCount = 0;
    clusterData.frameNumber = 0;
    clusterData.minClusterSize = MIN_CLUSTER_SIZE;
    clusterData.maxClusters = MAX_CLUSTERS_PER_FRAME;
    clusterData.clusters.resize(MAX_CLUSTERS_PER_FRAME);
    
    ofLogNotice("Simulator::setupClusterAnalysis") << "Cluster analysis initialized with min size: " << MIN_CLUSTER_SIZE 
              << ", connection distance: " << clusterConnectionDistance;
}

void Simulator::setupVACAnalysis() {
    velocityHistory.clear();
    velocityHistory.resize(maxVelocityFrames);
    
    // Initialize each frame's velocity storage for ensemble center-of-mass velocity (single vec2 per frame)
    for (auto& frame : velocityHistory) {
        frame.resize(1); // Only store one velocity per frame (the ensemble velocity)
    }
    
    vacData = VACData(); // Reset VAC data
    
    ofLogNotice("Simulator::setupVACAnalysis") << "VAC analysis initialized with " 
              << maxVelocityFrames << " frames history, using ensemble center-of-mass velocity";
}

void Simulator::update() {
    currentFrameNumber++;
    updateParticlesOnGPU();
    
    if (parameters->enableCollisionLogging) {
        readCollisionData();
        // Copy internal collision data to public collision data for external access
        collisionData = collisionBuffer;
    }
    
    // Perform cluster analysis if enabled (independent of collision logging)
    if (enableClusterAnalysis) {
        analyzeParticleClusters();
    }
    
    // Store velocity data and calculate VAC if enabled
    if (enableVACCalculation) {
        storeVelocityFrame();
        // Calculate VAC less frequently for better performance
        if ((currentFrameNumber % vacCalculationInterval) == 0) {
            calculateVAC();
        }
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
                // Copy collision data and normalize positions
                for (uint32_t i = 0; i < actualCollisions; i++) {
                    collisionBuffer.collisions[i] = collisionPtr[i];
                    // Normalize collision positions to
                    collisionBuffer.collisions[i].positionA = normalizePosition(collisionPtr[i].positionA);
                    collisionBuffer.collisions[i].positionB = normalizePosition(collisionPtr[i].positionB);
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
    
    // No need to update sampling since we're using ensemble velocity
    if (enableVACCalculation) {
        ofLogNotice("Simulator") << "VAC ensemble calculation updated for " << particles.active.size() << " particles";
    }
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
    
    case 'b':
    case 'B':
        // Decrease VAC calculation frequency (increase interval)
        vacCalculationInterval = std::min(60u, vacCalculationInterval + 5);
        ofLogNotice("Simulator") << "VAC calculation interval increased to " << vacCalculationInterval << " frames";
        break;
    case 'g':
    case 'G':
        // Increase VAC calculation frequency (decrease interval)
        vacCalculationInterval = std::max(5u, vacCalculationInterval - 5);
        ofLogNotice("Simulator") << "VAC calculation interval decreased to " << vacCalculationInterval << " frames";
        break;
    default:
        break;
    }
}

void Simulator::analyzeParticleClusters() {
    // Disjoint Set (Union-Find Data Structure)
    const uint32_t maxParticlesForAnalysis = 512; // Limit analysis to first 512 particles
    const uint32_t particleCount = std::min(static_cast<uint32_t>(particles.active.size()), maxParticlesForAnalysis);
    
    if (particleCount < MIN_CLUSTER_SIZE) {
        clusterData.clusterCount = 0;
        return;
    }
    
    
    // Initialize Union-Find data structures
    std::vector<uint32_t> parent(particleCount);
    std::vector<uint32_t> rank(particleCount, 0);
    
    // Initialize parent array - each particle is its own parent initially
    for (uint32_t i = 0; i < particleCount; i++) {
        parent[i] = i;
    }
    
    // Process collisions to build connected components
    performUnionFind(parent, rank);
    
    // Group particles by their root parent to form clusters
    std::vector<std::unordered_set<uint32_t>> clusterMembers;
    std::vector<uint32_t> rootToClusterIndex(particleCount, UINT32_MAX);
    
    // Find all unique roots and create cluster groups
    for (uint32_t i = 0; i < particleCount; i++) {
        uint32_t root = findRoot(parent, i);
        
        if (rootToClusterIndex[root] == UINT32_MAX) {
            rootToClusterIndex[root] = static_cast<uint32_t>(clusterMembers.size());
            clusterMembers.emplace_back();
        }
        
        clusterMembers[rootToClusterIndex[root]].insert(i);
    }
   
    
    // Filter clusters by minimum size and calculate statistics
    calculateClusterStatistics(parent, clusterMembers);
}

void Simulator::performUnionFind(std::vector<uint32_t>& parent, std::vector<uint32_t>& rank) {
    // Distance-based clustering for nearby particles (primary method)
    const uint32_t maxParticles = static_cast<uint32_t>(parent.size());
    const float connectionThresholdSq = clusterConnectionDistance * clusterConnectionDistance;
    uint32_t connectionsFound = 0;
    
    for (uint32_t i = 0; i < maxParticles; i++) {
        for (uint32_t j = i + 1; j < maxParticles; j++) {
            const Particle& p1 = particles.active[i];
            const Particle& p2 = particles.active[j];
            
            glm::vec2 diff = p1.position - p2.position;
            float distanceSq = glm::dot(diff, diff);
            
            // Use cluster connection distance as the only threshold
            if (distanceSq <= connectionThresholdSq) {
                unionSets(parent, rank, i, j);
                connectionsFound++;
            }
        }
    }
    
    if (currentFrameNumber % 60 == 0) {
        ofLogNotice("Simulator::ClusterDebug") << "Found " << connectionsFound << " particle connections using distance-based clustering only";
    }
}

uint32_t Simulator::findRoot(std::vector<uint32_t>& parent, uint32_t particle) {
    if (parent[particle] != particle) {
        parent[particle] = findRoot(parent, parent[particle]); // Path compression
    }
    return parent[particle];
}

void Simulator::unionSets(std::vector<uint32_t>& parent, std::vector<uint32_t>& rank, uint32_t a, uint32_t b) {
    uint32_t rootA = findRoot(parent, a);
    uint32_t rootB = findRoot(parent, b);
    
    if (rootA != rootB) {
        // Union by rank for better performance
        if (rank[rootA] < rank[rootB]) {
            parent[rootA] = rootB;
        } else if (rank[rootA] > rank[rootB]) {
            parent[rootB] = rootA;
        } else {
            parent[rootB] = rootA;
            rank[rootA]++;
        }
    }
}

void Simulator::calculateClusterStatistics(const std::vector<uint32_t>& parent, const std::vector<std::unordered_set<uint32_t>>& clusterMembers) {
    clusterData.frameNumber = currentFrameNumber;
    clusterData.clusterCount = 0;
    
    uint32_t clusterIndex = 0;
    
    for (const auto& cluster : clusterMembers) {
        // Only consider clusters with minimum required size
        if (cluster.size() >= MIN_CLUSTER_SIZE && clusterIndex < MAX_CLUSTERS_PER_FRAME) {
            ClusterStats& stats = clusterData.clusters[clusterIndex];
            
            stats.groupIndex = clusterIndex;
            stats.particleCount = static_cast<uint32_t>(cluster.size());
            stats.frameNumber = currentFrameNumber;
            
            // Calculate center position and average velocity
            glm::vec2 totalPosition(0.0f);
            glm::vec2 totalVelocity(0.0f);
            
            for (uint32_t particleId : cluster) {
                const Particle& particle = particles.active[particleId];
                totalPosition += particle.position;
                totalVelocity += particle.velocity;
            }
            
            glm::vec2 centerPosition = totalPosition / static_cast<float>(cluster.size());
            stats.averageVelocity = totalVelocity / static_cast<float>(cluster.size());
            
            // Calculate spatial spread (standard deviation of positions)
            float spatialVariance = 0.0f;
            float velocityVariance = 0.0f;
            
            for (uint32_t particleId : cluster) {
                const Particle& particle = particles.active[particleId];
                
                glm::vec2 positionDiff = particle.position - centerPosition;
                spatialVariance += glm::dot(positionDiff, positionDiff);
                
                glm::vec2 velocityDiff = particle.velocity - stats.averageVelocity;
                velocityVariance += glm::dot(velocityDiff, velocityDiff);
            }
            
            float spatialSpread = std::sqrt(spatialVariance / static_cast<float>(cluster.size()));
            stats.velocitySpread = std::sqrt(velocityVariance / static_cast<float>(cluster.size()));
            
            // normalize cluster data
            stats.centerPosition = normalizePosition(centerPosition);
            stats.spatialSpread = normalizeDistance(spatialSpread);
            
            clusterIndex++;
        }
    }
    
    clusterData.clusterCount = clusterIndex;
}

glm::vec2 Simulator::normalizePosition(const glm::vec2& position) {
    // convert world coordinates to normalized -1 to 1 range
    float normalizedX = (2.0f * position.x / static_cast<float>(width)) - 1.0f;
    float normalizedY = (2.0f * position.y / static_cast<float>(height)) - 1.0f;
    return glm::vec2(normalizedX, normalizedY);
}

float Simulator::normalizeDistance(float distance) {
    // normalize distance relative to the diagonal of the world space
    float worldDiagonal = std::sqrt(static_cast<float>(width * width + height * height));
    return (2.0f * distance / worldDiagonal);
}

void Simulator::storeVelocityFrame() {
    if (!enableVACCalculation || particles.active.empty()) return;

    uint32_t frameIndex = vacData.currentFrame % maxVelocityFrames;

    if (velocityHistory[frameIndex].size() != particles.active.size()) {
        velocityHistory[frameIndex].resize(particles.active.size());
    }

    for (size_t i = 0; i < particles.active.size(); i++) {
        velocityHistory[frameIndex][i] = particles.active[i].velocity;
    }

    vacData.currentFrame++;
}

void Simulator::calculateVAC() {
    if (!enableVACCalculation || vacData.currentFrame < 2 || particles.active.empty()) return;

    if ((currentFrameNumber - vacData.lastCalculationFrame) < vacCalculationInterval) return;
    vacData.lastCalculationFrame = currentFrameNumber;

    uint32_t availableFrames = std::min(vacData.currentFrame, maxVelocityFrames);
    uint32_t maxTimeLags = std::min(vacData.maxTimeLags, availableFrames - 1);

    // Reset VAC values
    std::fill(vacData.vacValues.begin(), vacData.vacValues.end(), 0.0f);

    // sum correlations across all particles
    for (uint32_t dt = 0; dt < maxTimeLags; dt++) {
        double correlation = 0.0;

        for (size_t particleIdx = 0; particleIdx < particles.active.size(); particleIdx++) {
            // Get current velocity (t=0)
            uint32_t currentFrameIdx = (vacData.currentFrame - 1) % maxVelocityFrames;
            if (velocityHistory[currentFrameIdx].size() <= particleIdx) continue;

            glm::vec2 v0 = velocityHistory[currentFrameIdx][particleIdx];

            // Get velocity at time dt ago
            uint32_t pastFrameIdx = (vacData.currentFrame - 1 - dt + maxVelocityFrames) % maxVelocityFrames;
            if (velocityHistory[pastFrameIdx].size() <= particleIdx) continue;

            glm::vec2 vt = velocityHistory[pastFrameIdx][particleIdx];

            correlation += glm::dot(v0, vt);
        }

        vacData.vacValues[dt] = static_cast<float>(correlation);
    }

    // Normalize so VAC(0) = 1
    if (vacData.vacValues[0] > 0.0f) {
        float normalizationFactor = 1.0f / vacData.vacValues[0];
        for (uint32_t i = 0; i < maxTimeLags; i++) {
            vacData.vacValues[i] *= normalizationFactor;
        }
    }
}
