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

    parameters->maxCollisionSampling = Simulator::MAX_COLLISIONS_PER_FRAME;
    parameters->maxClusterParticlesSampling = Simulator::MAX_CLUSTERS_PER_FRAME;

    /// track mixing
    collisionSampler1 >> collisionTrack;
    collisionSampler2 >> collisionTrack;
    clusterSampler1   >> clusterTrack;
    clusterSampler2   >> clusterTrack;
    clusterSampler3   >> clusterTrack;
    // todo: synths need to be patchable (to have moduleOutputs)
    //clusterSynth1     >> clusterTrack;
    //clusterDataSynth1 >> clusterTrack;
    noiseSynth        >> velocityTrack;
    ambientSampler    >> backgroundTrack;

    /// master mixing
    collisionTrack    >> master;
    clusterTrack      >> master;
    velocityTrack     >> master;
    backgroundTrack   >> master;

    /// final effects
    master >> panner; // to stereo
    panner >> masterCompressor.ch(0) >> audioEngine.audio_out(0);
    panner >> masterCompressor.ch(1) >> audioEngine.audio_out(1);
    //master >> lowEQ >> midEQ >> highEQ >> masterCompressor >> masterReverb;  // todo

    /// scope
    collisionTrack   >> collisionScope  >> audioEngine.blackhole();
    clusterTrack     >> clustersScope   >> audioEngine.blackhole();
    velocityTrack    >> velocityScope >> audioEngine.blackhole();
    backgroundTrack  >> backgroundScope >> audioEngine.blackhole();

    /// sound setup
    audioEngine.listDevices();
    audioEngine.setDeviceID(0); // todo: add control to change this from gui (currently uses the system's default interface)
    audioEngine.setup(44100, 512, 6);

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
    ofSetColor(255, 255, 255, 60);
    collisionScope.draw(0,  scopeHeight * 0, ofGetWidth(), scopeHeight);
    clustersScope.draw(0,   scopeHeight * 1, ofGetWidth(), scopeHeight);
    velocityScope.draw(0,   scopeHeight * 2, ofGetWidth(), scopeHeight);
    backgroundScope.draw(0, scopeHeight * 3, ofGetWidth(), scopeHeight);
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
// sonifications
// 
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
    if (parameters->particlesInClusterRate > 0.5) {
        playClusterSounds();
    }

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

    auto factorRanges = [](int n) {
        if (n >= 1 && n <= 300)   return ofMap(n, 1, 300, 1, 1.5);
        if (n >= 301 && n <= 600) return ofMap(n, 301, 600, 10, 20);
        if (n >= 601 )            return ofMap(n, 601, 1000, 30, 60, true);
        };
    
    // it is too excitable now
    // the ranges should change: we must use presets with fewer particles
    // bellow 50 particles, each individual collision should sound as if they are triggering at real-time (with a limit of 3 per time) and not stealing
    // between 50 and 300 rythms can emerge
    // avobe that, they should trigger only once in a while, very spaced, very rarely

    frequencyFactor = factorRanges(activeParticles);
    float intervalA = 0.3 * frequencyFactor;
    float intervalB = 6.0 * frequencyFactor;

    //bool gate = ofRandomGaussian(0., 1.) < (parameters->collisionRate * parameters->collisionRate) / 2;
    bool gate       = ofNoise(ofGetElapsedTimef()) < parameters->collisionRate * 2;
    bool bypassGate = ofNoise(ofGetElapsedTimeMillis()) < parameters->collisionRate * 2;

    //triggerAtInterval(intervalB, [&]() {
    //    if (gate) {
    //        collisionSampler2.play(pitch, 0);
    //    }
    //    });
    triggerAtInterval(2.0, [&]() {
        if (gate || bypassGate) {
            collisionSampler1.play(pitch, 0, false);
            ofLog() << "playing collision sample pitch " << pitch;
        }
        });

    //triggerAtGate({ {0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, {0.1f, 0.0f} }, 4.0f, [&]() {
    //        collisionSampler1.play(pitch, 0);
    //    });

}
void AudioApp::stopCollisionSounds() {
    collisionSampler1.stop();
    collisionSampler2.stop();
}
void AudioApp::setupCollisionSounds(int bank) {
    switch(bank){
    default:
    case 0:
        collisionSampler1.add(ofToDataPath("sounds/kalimba.wav"));
        collisionSampler1.setReverb(0.5, 0.3, 2.0, 0.3, 0., 0.);
        //collisionSampler1.setDelay(-1, 0.3f, 0.2f, 0.7);

        collisionSampler2.add(ofToDataPath("sounds/obell-c.wav"));
        collisionSampler2.setReverb(1.0, 0.3, 10.0, 0.6, 0., 0.);
        collisionSampler2.setDelay(1, 0.15f, 0.6f, 0.5);
        break;
    }
}
#pragma endregion collisions


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
    float pitch = 45;
    float reso = 0.6f;

    // map the pitch and filter mode to the vac value
    if (parameters->vacValues.get().size() > 20) {
        pitch = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 2)], 0.0, 1.0, 30, 80);
        reso  = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 4)], 0.0, 1.0, 0.4, 0.8);
    }

    // when thermostat is on, use bandpass filter; otherwise use highpass
    int mode = allParameters->simulationParameters.applyThermostat.get() ? pdsp::VAFilter::BandPass24 : pdsp::VAFilter::BandPass12;

    float volume = ofMap(parameters->collisionRate.get(), 0.00, 0.20, 0.0, 1.0, true);

    noiseSynth.setFilter(mode, pitch, reso);
    noiseSynth.play(volume);
}
void AudioApp::stopVelocityNoise() {
    noiseSynth.stop();
}


/// cohesives
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playClusterSounds() {
    float pitch = 45;
    float reso = 0.6f;

    // map the pitch and filter mode to the vac value
    if (clusterData->clusters.size() > 0) {
        float velMag = glm::length(clusterData->clusters[0].averageVelocity);
        pitch = ofMap(velMag, 0.0, 100.0, 45, 80, true);
    }

    // when thermostat is on, use bandpass filter; otherwise use highpass
    int mode = 4; // allParameters->simulationParameters.applyThermostat.get() ? 4 : 0;

    float volume = ofMap(parameters->avgClusterVelocity.get(), 5, 50, -30.0, 0.0, true);

    clusterSampler1.filterPitchControl.set(pitch);
    //clusterSampler1.setFilter(mode, 0.3, pitch);
    clusterSampler1.play(0.0, 0, false);
    clusterSampler1.fader.set(volume);
}
void AudioApp::stopClusterSounds() {
    clusterSampler1.stop();
    //clusterSampler2.stop();
    //clusterSampler3.stop();
}
void AudioApp::setupClusterSounds(int bank) {
    switch (bank) {
    default:
    case 0:
        clusterSampler1.add(ofToDataPath("sounds/bg-ch1m.wav"));
        clusterSampler1.add(ofToDataPath("sounds/endless_strings.aif.wav"));
        clusterSampler1.envSmoothControl.set(1300);
        clusterSampler1.setReverb(0.5, 0.3, 6.0, 0.8, 0., 0.);
        clusterSampler1.setDelay(-40, 0.1f, 0.4f, 0.5);
        clusterSampler1.setFilter(4, 0.3, 60);
        break;
    }
}



/// ambient sounds that triggers once in a while in the background to fill the space and add variety
/// ---------------------------------------------------------------------------------------------------
void AudioApp::setupAmbientSounds(int bank) {
    switch (bank) {
    default:
    case 0:
        ambientSampler.add(ofToDataPath("sounds/bg-ch1m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch2m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch3m.wav"));
        ambientSampler.add(ofToDataPath("sounds/bg-ch4m.wav"));
        clusterSampler1.setFilter(0,0,120); 
        break;
    }
}

void AudioApp::playAmbientSounds() {
    if (allParameters->simulationParameters.amount.get() < 30) {
        ambientSampler.play(0, (int)ofRandom(0, 4));
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





