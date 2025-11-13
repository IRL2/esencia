#include "AudioApp.h"
#include "../simulation/simulator.h"





void AudioApp::setup(SonificationParameters *params, GuiApp* allParams) {
    ofLogNotice("AudioApp::setup") << "Audio application initialized";

    parameters = params;
    allParameters = allParams;
    
    audioEnabled = true;
    clusterAnalysisEnabled = true;
    lastProcessedFrame = 0;
    lastProcessedClusterFrame = 0;
    DEBUG_LOG = false;
    SCOPE_COLOR = ofColor(255, 255, 255, 60);

    parameters->maxCollisionSampling = Simulator::MAX_COLLISIONS_PER_FRAME;
    parameters->maxClusterParticlesSampling = Simulator::MAX_CLUSTERS_PER_FRAME;

    /// track mixing
    collisionSampler1 >> collisionTrack;
    collisionSampler2 >> collisionTrack;
    // todo: synths need to be patchable (to have moduleOutputs)
    //clusterSynth1     >> clusterTrack;
    //clusterDataSynth1 >> clusterTrack;
    noiseSynth        >> velocityTrack;
    ambientSampler    >> backgroundTrack;
    clusterSampler.resize(CLUSTER_SOUNDS_SIZE);
    for (auto& c : clusterSampler) { c >> clusterTrack; }

    /// master mixing
    collisionTrack    >> master;
    clusterTrack      >> master;
    velocityTrack     >> master;
    backgroundTrack   >> master;

    /// final effects
    //master >> masterReverb >> panner;
    master >> panner;
    panner >> masterCompressor.ch(0) >> audioEngine.audio_out(0);
    panner >> masterCompressor.ch(1) >> audioEngine.audio_out(1);
    //master >> lowEQ >> midEQ >> highEQ >> masterCompressor >> masterReverb;  // todo
    -10.0f >> masterCompressor.in_threshold();
    10.0f >> masterCompressor.in_attack();
    100.0f >> masterCompressor.in_release();
    3.0f >> masterReverb.in_time();
    0.9f >> masterReverb.in_damping();
    5000.0f >> masterReverb.in_hi_cut();

    /// scope
    collisionTrack   >> collisionScope  >> audioEngine.blackhole();
    clusterTrack     >> clustersScope   >> audioEngine.blackhole();
    velocityTrack    >> velocityScope >> audioEngine.blackhole();
    backgroundTrack  >> backgroundScope >> audioEngine.blackhole();

    /// sound setup
    audioEngine.listDevices();
    audioEngine.setDeviceID(0); // todo: add control to change this from gui (currently uses the system's default interface)
    audioEngine.setup(44100, 512, 6);

    /// instrument setups (load samples, set fx, etc), important step!
    setupVelocityNoise();
    setupCollisionSounds(0);
    setupClusterSounds(0);
    setupAmbientSounds(0);

    ofAddListener(ofEvents().windowResized, this, &AudioApp::windowResize);
    windowResize(ofResizeEventArgs());
}





void AudioApp::update() {

    // saving reference of the last second for some time-based triggers
    if (ofGetUnixTime() != lastTime) {
        lastTime = ofGetUnixTime();
    }
    
    
    // Process collision data
    if (collisionData != nullptr) {
        if (collisionData->frameNumber > lastProcessedFrame && collisionData->collisionCount > 0) {
            if (DEBUG_LOG) { logCollisionDetails(*collisionData); }
            processCollisionsStatistics(*collisionData);
            lastProcessedFrame = collisionData->frameNumber;
        }
        if (collisionData->collisionCount == 0) {
            cleanCollisionStatistics();
        }
    }

    // Process cluster data
    if (clusterData != nullptr) {
        if (clusterData->frameNumber > lastProcessedClusterFrame && clusterData->clusterCount > 0) {
            if (DEBUG_LOG) { logClusterDetails(*clusterData); }
            processClusterStatistics(*clusterData);
            lastProcessedClusterFrame = clusterData->frameNumber;
        }
        if (clusterData->clusterCount == 0) {
            cleanClusterStatistics();
        }
    }


    // main audio control function
    sonificationControl(*collisionData, *clusterData);


    // never forget to put a mute button
    if (ofGetKeyPressed('m') || ofGetKeyPressed('M')) {
        parameters->masterVolume = 0.0f;
    }


    if (ofGetKeyPressed() == 'c' || ofGetKeyPressed() == 'C') {
        collisionSampler1.play(0,0,true,1.0);
    };
}

// just to keep the audio scopes sized properly on window resize
void AudioApp::windowResize(ofResizeEventArgs&) {
    scopeHeight = (ofGetHeight() / 4);
}


void AudioApp::draw() {
    ofPushStyle();
    drawScope(collisionScope, 0, scopeHeight * 0, ofGetWidth(), scopeHeight);
    drawScope(clustersScope, 0, scopeHeight * 1, ofGetWidth(), scopeHeight);
    drawScope(velocityScope, 0, scopeHeight * 2, ofGetWidth(), scopeHeight);
    drawScope(backgroundScope, 0, scopeHeight * 3, ofGetWidth(), scopeHeight);
    ofPopStyle(); 
}


// from the original pdsp::Scope draw method, without the frame
void AudioApp::drawScope(pdsp::Scope &s, int x, int y, int w, int h) const {
    ofPushStyle();
    ofPushMatrix();
    ofTranslate(x, y);
    ofNoFill();
    ofSetLineWidth(1);
    ofSetColor(SCOPE_COLOR);

    int bufferLen = s.getBuffer().size();

    float xMult = (float)bufferLen / (float)w;
    float yHalf = h / 2;
    float yMult = -yHalf;

    ofBeginShape();
    for (int xx = 0; xx < w; xx++) {
        int index = xx * xMult;
        float value = s.getBuffer()[index];
        ofVertex(xx, yHalf + value * yMult);
    }
    ofEndShape(false);

    ofPopMatrix();
    ofPopStyle();
}


/// <summary>
/// console logs the details of the clusters detected in the current frame
/// nothing is processed here
/// </summary>
/// <param name="clusterData"></param>
void AudioApp::logClusterDetails(const ClusterAnalysisData& clusterData) {
    if (clusterData.clusterCount == 0) return;

    ofLogNotice("AudioApp::ClusterData") << "Frame " << clusterData.frameNumber
        << ": Processing " << clusterData.clusterCount << " clusters for audio analysis";

    // Log details for each cluster (limit to first 5 to avoid spam)
    uint32_t logLimit = std::min(clusterData.clusterCount, static_cast<uint32_t>(5));
    for (uint32_t i = 0; i < logLimit; i++) {
        const ClusterStats& cluster = clusterData.clusters[i];

        ofLogNotice("AudioApp::ClusterDetails") << "  Cluster " << (cluster.groupIndex + 1)
            << ": " << cluster.particleCount << " particles"
            << " | Center: (" << std::fixed << std::setprecision(1) 
            << cluster.centerPosition.x << ", " << cluster.centerPosition.y << ")"
            << " | Spatial Spread: " << std::fixed << std::setprecision(2) << cluster.spatialSpread
            << " | Avg Velocity: (" << std::fixed << std::setprecision(1) 
            << cluster.averageVelocity.x << ", " << cluster.averageVelocity.y << ")"
            << " | Velocity Spread: " << std::fixed << std::setprecision(2) << cluster.velocitySpread;
    }
    if (clusterData.clusterCount > 5) {
        ofLogNotice("AudioApp::ClusterDetails") << "  ... and " << (clusterData.clusterCount - 5) << " more clusters";
    }
}


/// <summary>
/// display collision details in the console for debugging; no processing
/// </summary>
void AudioApp::logCollisionDetails(const CollisionBuffer& collisionData) {
    if (collisionData.collisionCount == 0) return;

    uint32_t actualCollisions = std::min((int)collisionData.collisionCount, parameters->maxCollisionSampling.get());

    ofLogNotice("AudioApp::CollisionData") << "Frame " << collisionData.frameNumber
        << ": Processing " << actualCollisions << " collisions for audio";

    // Log details for each collision (limit to first 10 to avoid spam)
    uint32_t logLimit = std::min(actualCollisions, static_cast<uint32_t>(10));
    for (uint32_t i = 0; i < logLimit; i++) {
        const CollisionData& collision = collisionData.collisions[i];
        if (collision.valid) {
            ofLogNotice("AudioApp::CollisionDetails") << "  Audio Processing Collision " << (i + 1)
                << ": Particles " << collision.particleA << " & " << collision.particleB
                << " | Distance: " << std::fixed << std::setprecision(2) << collision.distance
                << " | Velocity Magnitude: " << std::fixed << std::setprecision(2) << collision.velocityMagnitude
                << " | Pos A: (" << std::fixed << std::setprecision(1) << collision.positionA.x << ", " << collision.positionA.y << ")"
                << " | Pos B: (" << std::fixed << std::setprecision(1) << collision.positionB.x << ", " << collision.positionB.y << ")";
        }
    }

    if (actualCollisions > 10) {
        ofLogNotice("AudioApp::CollisionDetails") << "  ... and " << (actualCollisions - 10) << " more collisions being processed for audio";
    }
}



/// <summary>
/// post-processing of the physics analysis on clusters: velocities, sizes, positions
/// preparing data for the actual sonification step
/// </summary>
/// <param name="clusterData"></param>
void AudioApp::processClusterStatistics(const ClusterAnalysisData& clusterData) {
    if (!clusterAnalysisEnabled || clusterData.clusterCount == 0) return;
    
    // Statistical analysis for audio processing
    float totalParticlesInClusters = 0;
    float averageClusterSize = 0;
    float totalSpatialSpread = 0;
    float totalVelocityMagnitude = 0;
    glm::vec2 centerOfMass(0.0f);

    //sort clusters by size descending
    std::vector<ClusterStats>* clustersPtr = const_cast<std::vector<ClusterStats>*>(&clusterData.clusters);
    std::sort(clustersPtr->begin(), clustersPtr->end(), [](const ClusterStats& a, const ClusterStats& b) {
        return a.particleCount > b.particleCount;
        });

    // Calculate aggregate statistics
    for (uint32_t i = 0; i < clusterData.clusterCount; i++) {
        const ClusterStats& cluster = clusterData.clusters[i];
        
        totalParticlesInClusters += cluster.particleCount;
        totalSpatialSpread += cluster.spatialSpread;
        
        float velocityMagnitude = glm::length(cluster.averageVelocity);
        totalVelocityMagnitude += velocityMagnitude;
        
        // Weight center of mass by cluster size
        centerOfMass += cluster.centerPosition * static_cast<float>(cluster.particleCount);
    }
    
    //if (clusterData.clusterCount > 0) {
        averageClusterSize = totalParticlesInClusters / static_cast<float>(clusterData.clusterCount);
        centerOfMass /= totalParticlesInClusters;
        
        float averageSpatialSpread = totalSpatialSpread / static_cast<float>(clusterData.clusterCount);
        float averageVelocityMagnitude = totalVelocityMagnitude / static_cast<float>(clusterData.clusterCount);

        // Log aggregate statistics for audio processing
        if (DEBUG_LOG) ofLogNotice("AudioApp::ClusterStatistics") << "Frame " << clusterData.frameNumber << " Audio Processing:"
            << " | Clusters: " << clusterData.clusterCount
            << " | Avg Size: " << std::fixed << std::setprecision(1) << averageClusterSize
            << " | Total Particles: " << static_cast<uint32_t>(totalParticlesInClusters)
            << " | Center of Mass: (" << std::fixed << std::setprecision(1) << centerOfMass.x << ", " << centerOfMass.y << ")"
            << " | Avg Spatial Spread: " << std::fixed << std::setprecision(2) << averageSpatialSpread
            << " | Avg Velocity: " << std::fixed << std::setprecision(2) << averageVelocityMagnitude;
        
        for (uint32_t i = 0; i < clusterData.clusterCount; i++) {
            const ClusterStats& cluster = clusterData.clusters[i];
			// placeholder for audio processing logic
        }
    //}


    // update the summarized data to the parameters for gui display
    parameters->clusters.set((float)clusterData.clusterCount);
    parameters->particlesInClusters.set((float)totalParticlesInClusters);
    parameters->avgClusterSize.set(averageClusterSize);
    parameters->avgClusterVelocity.set(totalVelocityMagnitude / static_cast<float>(clusterData.clusterCount));
    parameters->particlesInClusterRate.set(totalParticlesInClusters / fminf(allParameters->simulationParameters.amount, Simulator::MAX_PARTICLES_FOR_CLUSTER_ANALYSIS));
    parameters->avgClusterSpatialSpread.set(averageSpatialSpread);
    parameters->avgClusterVelocityMagitude.set(averageVelocityMagnitude);
}

void AudioApp::cleanClusterStatistics() {
    parameters->clusters.set(0);
    parameters->particlesInClusters.set(0);
    parameters->avgClusterSize.set(0);
    parameters->avgClusterVelocity.set(0);
    parameters->particlesInClusterRate.set(0);
    parameters->avgClusterSpatialSpread.set(0);
    parameters->avgClusterVelocityMagitude.set(0);
}

void AudioApp::cleanCollisionStatistics() {
    parameters->collisionRate.set(0);
    parameters->collisions.set(0);
}


/// <summary>
/// post-processing of the physics analysis: counting collisions
/// preparing data for the actual sonification step
/// </summary>
/// <param name="collisionData"></param>
void AudioApp::processCollisionsStatistics(const CollisionBuffer& collisionData) {
    //if (collisionData.collisionCount == 0) return;

    int actualEvaluations = std::min((int)allParameters->simulationParameters.amount.get(), parameters->maxCollisionSampling.get());
    int actualCollisions = std::min((int)collisionData.collisionCount, actualEvaluations);

    parameters->collisionRate.set(static_cast<float>(actualCollisions) / actualEvaluations);
    parameters->collisions.set(static_cast<float>(actualCollisions));
}





//
// -----------------------------------------------------------------------------------------
// sonification code
// 


/// <summary>
/// sonification based on collision and cluster data
/// **also make use of the sonification parameters data
/// </summary>
/// <param name="collisionData"></param>
/// <param name="clusterData"></param>
void AudioApp::sonificationControl(const CollisionBuffer& collisionData, const ClusterAnalysisData& clusterData) {
    if (!audioEnabled) return;

    // update volumes from the gui (via parameters)
    master.set(parameters->masterVolume);
    collisionTrack.set(parameters->collisionVolume);
    clusterTrack.set(parameters->clusterVolume);
    velocityTrack.set(parameters->velocityVolume);
    backgroundTrack.set(parameters->backgroundVolume);

    /// filler ambient
    playAmbientSounds();

    /// forming clusters
    if (parameters->particlesInClusterRate > 0.3 && parameters->clusters > 0) {
        playClusterSounds();
    }
    else { stopClusterSounds(); }

    /// background sound, when nothing clear happens
    playVelocityNoise();
    if (!allParameters->sonificationParameters.enableVACCalculation.get()) stopVelocityNoise();

    /// collision sounds when few particles are there and collides
    playCollisionSounds(1.0);
}


/// collisions
#pragma region collisions
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playCollisionSounds(float frequencyFactor) {

    int notes[10] = { -2, 0, 2, 4, 7, 1, 3, 5, 0, 4 };
    int noteShift = lastTime % 2 == 0 ? 0 : 5;
    //int pitch = notes[(int)ofMap(parameters->collisionRate, 0, 1.3, noteShift, noteShift + 4)];
    int pitch = notes[(int)ofMap(ofNoise(ofGetElapsedTimeMillis()), 0, 1, noteShift, noteShift + 4)];

    int activeParticles = allParameters->simulationParameters.amount.get();

    static float intervalA = 1.0; // individual collisions
    static float intervalB = 6.0; // ambient collisions

    bool gate = ofNoise(ofGetElapsedTimef()) < parameters->collisionRate;
    gate = ofRandomuf() + parameters->collisionRate > 0.7;

    // switch sample depending on the number of particles
    static int sample = 0;
    if (activeParticles <= 100) {
        sample = 0;
        intervalA = 0.25;
    }
    else if (activeParticles > 100 && activeParticles <= 600) {
        sample = 1;
        intervalA = 0.75;
    }
    else {
        sample = 2;
        intervalA = 2;
        gate = ofRandomf() < 0.5;
    }
    
    triggerAtInterval(intervalA, [&]() {
        if (gate) {
            // trigger the waterdrops if the collision rate is high enough
            if (parameters->collisionRate.get() > 0.8) {
                collisionSampler2.play(0, (int)ofRandom(collisionSampler2.samples.size()));
            }
            else {
                collisionSampler1.play(pitch, sample, false);
            }
        }
        });

    // secondary layer, melodic waterdrops, triggered at random times
    triggerAtInterval(intervalB, [&]() {
        //collisionSampler2.play(0, (int)ofRandom(4));
        });
    // every 5 repetitions, change the interval randomly
    triggerAtInterval(intervalB*5, [&]() {
        intervalB = (int) ceil(intervalB * (ofRandom(0.5, 2.0)));
        intervalB = clamp(intervalB, 6.0f, 25.0f);
        });


}
void AudioApp::stopCollisionSounds() {
    collisionSampler1.stop();
    collisionSampler2.stop();
}
void AudioApp::setupCollisionSounds(int bank) {
    switch(bank){
    default:
    case 0:
        collisionSampler1.add(ofToDataPath("sounds/collision-drop1.wav"));
        collisionSampler1.add(ofToDataPath("sounds/collision-shft1.wav"));
        collisionSampler1.add(ofToDataPath("sounds/collision-dub2.wav"));
        collisionSampler1.setReverb(0.5, 0.3, 2.0, 0.3, 0., 0.);
        collisionSampler1.setDelay(1, 0.3f, 0.2f, 0.7);

        collisionSampler2.add(ofToDataPath("sounds/collision-arp-drops1.wav"));
        collisionSampler2.add(ofToDataPath("sounds/collision-arp-drops2.wav"));
        collisionSampler2.add(ofToDataPath("sounds/collision-arp-drops3.wav"));
        collisionSampler2.add(ofToDataPath("sounds/collision-arp-drops4.wav"));
        break;
    }
}
#pragma endregion collisions


#pragma region velocityNoise
/// background noise that responds to the VAC
/// useful for chaotic scenes: when many particles moves in crazy ways
/// or super calm scenes: many particles in screen, fewer moving (no thermostat)
/// ---------------------------------------------------------------------------------------------------
/// 
void AudioApp::setupVelocityNoise() {
    noiseSynth.setReverb(16, 0.17, 0.8, 0.2, 0.5);
    noiseSynth.setFilter(4, 60, 0.6);
    noiseSynth.reverbDryGain.set(0.5f);
    noiseSynth.amp.set(1.0f);
}
void AudioApp::playVelocityNoise() {
    float freq = 45;
    float reso = 0.6f;

    // map the pitch and filter mode to the vac value
    if (parameters->vacValues.get().size() > 20) {
        freq = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 2)], 0.0, 1.0, 30, 80);
        reso  = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 4)], 0.0, 1.0, 0.4, 0.8);
    }

    // when thermostat is on, use bandpass filter; otherwise use highpass
    int mode = allParameters->simulationParameters.applyThermostat.get() ? pdsp::VAFilter::BandPass24 : pdsp::VAFilter::BandPass12;

    float volume = ofMap(parameters->collisionRate.get(), 0.00, 0.20, 0.0, 1.0, true);

    noiseSynth.setFilter(mode, freq, reso);
    noiseSynth.play(volume);
}
void AudioApp::stopVelocityNoise() {
    noiseSynth.stop();
}
#pragma endregion velocityNoise


const float CLUSTER_VOLUME_SLOPE = 0.0333;
const int   CLUSTER_VOLUME_MEAN = 200;

#pragma region clusters
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playClusterSounds() {
    static float clustersToProcess = min(clusterData->clusters.size(), clusterSampler.size());
    for (int i = 0; i < clustersToProcess; ++i) {
        ClusterStats &cd = clusterData->clusters[i % clusterData->clusters.size()];
        AudioSampler &cs = clusterSampler[i % clusterSampler.size()];
        
        float reso = 0.6f;
        float velMag = glm::length(cd.averageVelocity);
        
        float filterFreq = 45;
        filterFreq = ofMap(velMag, 0.0, 200.0, 90, 45, true);
    
        int mode = 4; // allParameters->simulationParameters.applyThermostat.get() ? 4 : 0;

        // cluster volume is a logistic curve of the average velocity of the cluster
        // clusters are constantly moving slowly, people's force fields push the movement avobe 80 (so 200 is a good mean)
        float volume = 1 / (1 + (exp(CLUSTER_VOLUME_MEAN - velMag) * CLUSTER_VOLUME_SLOPE));

        // change pitch based on spatial spread of the cluster only when forcefields are repulsive
        float pitch = (allParameters->simulationParameters.depthFieldScale.get() > 0) ? ofMap(cd.spatialSpread, 0.0, 100.0, 2, 4, true) : 0;

        cs.filterPitchControl.set(filterFreq);
        cs.play(pitch, 0, false, volume);
        cs.amp.set(volume);
    }
}
void AudioApp::stopClusterSounds() {
    for (auto &c : clusterSampler) { c.stop(); }
}
void AudioApp::setupClusterSounds(int bank) {
    switch (bank) {
    default:
    case 0:
        for (int i = 0; i < clusterSampler.size(); ++i) {
            clusterSampler[i].add(ofToDataPath("sounds/cluster-c" + std::to_string(i + 1) + ".wav"));
            clusterSampler[i].setReverb(0.5, 0.3, 2.0, 0.3, 0., 0.);
            clusterSampler[i].setFilter(4, 0.3, 120);
        }
        break;
    }
}
#pragma endregion clusters



/// ambient sounds that triggers once in a while in the background to fill the space and add variety
/// ---------------------------------------------------------------------------------------------------
void AudioApp::setupAmbientSounds(int bank) {
    switch (bank) {
    default:
    case 0:
        ambientSampler.add(ofToDataPath("sounds/bg-ch1m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch4m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch3m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch2m.wav"));
        break;
    }
}

void AudioApp::playAmbientSounds() {
    if (allParameters->simulationParameters.amount.get() < 30) {
        ambientSampler.play(0, (int)ofRandom(0, ambientSampler.getNumSamples()));
        ambientSampler.fader.set(0.4);
    }
    triggerAtInterval(80.0f, [&]() {
        int sample = (int)ofRandom(0, 0);
        float volume = ofRandom(-10, 1);
        ambientSampler.play(sample, 0, false);
        ambientSampler.fader.set(volume);
        ofLog() << "playing ambient sample " << sample << " with volume " << volume;

        });
}
void AudioApp::stopAmbientSounds() {
    ambientSampler.stop();
}



/// stop all
/// ---------------------------------------------------------------------------------------------------
void AudioApp::stopAll() {
    stopCollisionSounds();
    stopVelocityNoise();
    stopClusterSounds();
}



#pragma region tools

/// <summary>
///  returns true if the specified interval in seconds has passed since the last check
/// </summary>
/// <param name="intervalInSeconds"></param>
/// <returns></returns>
bool AudioApp::checkInterval(float intervalInSeconds) {
    static std::map<float, uint64_t> lastCheckTimes;

    // Get current time
    uint64_t currentTimeMs = ofGetUnixTimeMillis();

    // Convert interval to milliseconds
    uint64_t intervalMs = static_cast<uint64_t>(intervalInSeconds * 1000);

    // Initialize if this is the first check for this interval
    if (lastCheckTimes.find(intervalInSeconds) == lastCheckTimes.end()) {
        lastCheckTimes[intervalInSeconds] = currentTimeMs;
        return false;
    }

    // Check if enough time has passed
    if (currentTimeMs - lastCheckTimes[intervalInSeconds] >= intervalMs) {
        // Update the last check time
        lastCheckTimes[intervalInSeconds] = currentTimeMs;
        return true;
    }

    return false;
}



/// <summary>
/// triggers a callback function at specified time intervals in seconds.
/// </summary>
/// <param name="intervalInSeconds">interval in seconds, at which to trigger the callback.</param>
/// <param name="callback">function to execute when the interval has elapsed: App::callback()</param>
/// <param name="callback">lambda function to execute when the interval has elapsed: [&]</param>
/// <returns>True if the callback was triggered during this call; false otherwise.</returns>
bool AudioApp::triggerAtInterval(float intervalInSeconds, std::function<void()> callback) {
    static std::map<float, uint64_t> lastTriggerTimes;

    // Get current time
    uint64_t currentTimeMs = ofGetUnixTimeMillis();

    // Convert interval to milliseconds
    uint64_t intervalMs = static_cast<uint64_t>(intervalInSeconds * 1000);

    // Check if this specific interval has a recorded last time
    if (lastTriggerTimes.find(intervalInSeconds) == lastTriggerTimes.end()) {
        lastTriggerTimes[intervalInSeconds] = currentTimeMs;
        return false;
    }

    // Check if enough time has passed
    if (currentTimeMs - lastTriggerTimes[intervalInSeconds] >= intervalMs) {
        // Execute the callback function
        callback();

        // Update the last trigger time
        lastTriggerTimes[intervalInSeconds] = currentTimeMs;
        return true;
    }

    return false;
}


#pragma endregion tools



