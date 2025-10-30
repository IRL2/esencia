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
    whiteNoise        >> backgroundTrack;
    ambienceSampler   >> backgroundTrack;

    /// master mixing
    collisionTrack    >> master;
    clusterTrack      >> master;
    backgroundTrack   >> master;

    /// final effects
    master.ch(0) >> masterCompressor.ch(0) >> audioEngine.audio_out(0);
    master.ch(1) >> masterCompressor.ch(1) >> audioEngine.audio_out(1);
    //master >> lowEQ >> midEQ >> highEQ >> masterCompressor >> masterReverb;  // todo

    /// scope
    masterCompressor >> mainScope       >> audioEngine.blackhole();
    collisionTrack   >> collisionScope  >> audioEngine.blackhole();
    clusterTrack     >> clustersScope   >> audioEngine.blackhole();
    backgroundTrack  >> backgroundScope >> audioEngine.blackhole();

    /// sound setup
    audioEngine.listDevices();
    audioEngine.setDeviceID(0); // todo: add control to change this from gui (currently uses the system's default interface)
    audioEngine.setup(44100, 512, 3);

    setupBackgroundNoise();
    setupCollisionSounds(0);
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

    scopeHeight = (ofGetHeight() / 4);
}


void AudioApp::draw() {
    ofPushStyle();
    ofSetColor(255, 255, 255, 70);
    mainScope.draw(-1, -1, ofGetWidth() + 2, scopeHeight);
    collisionScope.draw(-1, scopeHeight, ofGetWidth() + 2, scopeHeight);
    clustersScope.draw(-1, scopeHeight*2, ofGetWidth() + 2, scopeHeight);
    backgroundScope.draw(-1, scopeHeight*3, ofGetWidth()+2, scopeHeight);
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
    backgroundTrack.set(parameters->backgroundVolume);


    ///// zero particles
    //if (allParameters->simulationParameters.amount.get() == 0) {
    //    stopAll();
    //    playEmpty();
    //}
    //else {
    //    stopEmpty();
    //}


    ///// forming clusters
    //if (parameters->clusters > 1) {
    //    playCohesive();

    //    if (parameters->particlesInClusterRate.get() < 0.9) {
    //        playCollisionSounds();
    //    }
    //    else {
    //        stopCollisionSounds();
    //    }
    //}

    /// background sound, when nothing clear happens
    playBackgroundNoise();

    /// collision sounds when few particles are there and collides
    playCollisionSounds(1.0);

    //if (allParameters->simulationParameters.amount.get() > 150) {
    //    playBackgroundNoise();
    //} else {
    //    //stopChaotic();
    //}




    // when no data is presented
    //if (parameters->clusters == 0 || parameters->collisions == 0) {
    //    parameters->polysynthVolume.set(0.1);
    //    polySynth.setPitch(46);
    //    polySynth.setLFOfreq(0);
    //    return;
    //}
    //else {
    //    parameters->polysynthVolume.set(0.6);
    //}




    //if (sampler1.meter_position() > 0.0f && sampler1.meter_position() < 1.0) {
    //    //float resonanceTime = ofMap(parameters->clusters * parameters->collisionRate, 0.0, parameters->clusters, 0, 4);
    //    //sampler1.timeControl.set(resonanceTime);
    //    sampler1.modAmountControl.set(parameters->particlesInClusterRate * 2.0);
    //    //sampler1.pitchControl.set(ofMap(parameters->clusters, 1, 5, 0.3f, 1.5f));
    //}
    //else {
    //    //sampler1.play(parameters->particlesInClusterRate + 0.5f, ofClamp(parameters->clusters / 10, 0.7, 1.0));
    //    //float pitch = ofMap(parameters->collisionRate * parameters->clusters, 0.0, parameters->clusters, 0.2, 1.0);
    //    sampler1.play(1.0, 2.0); // prev
    //}

    // silent hill theme
    //sampler1.play(clusterData.clusterCount, cluster.particleCount);
    // melodic silent hill
    //sampler1.play(clusterData.clusterCount / 10, parameters->avgClusterSize);


    //// THE GOOD ONE tHAt WORKS NICE
    //-----------------------
    //size_t samplerIndex = sampler2.currentSampleIndex;
    //triggerAtInterval(2.0, [&]() {
    //    samplerIndex = (samplerIndex + 1) % sampler2.getNumSamples();
    //    sampler2.switchSampleIndex(samplerIndex);
    //    });

    //// this actually works well
    //int notesA[5] = { -2, 0, 2, 4, 7 };
    //int notesB[5] = { 1, 3, 5, 0, 4 };
    //int notes[] = { 0,0,0,0,0 };
    //std::copy(notesA, notesA + 5, notes);
    //if (lastTime % 2 == 0) {
    //    std::copy(notesB, notesB + 5, notes);
    //}
    //int pitch = notes[(int)ofMap(parameters->collisionRate, 0, 1.3, 0, 4)];

    //float vol = ofMap(parameters->sampler1playerVolume, 0.0, 1.0, -30, 5);

    //if (checkInterval(1.0)) {
    //    if (ofRandomGaussian(0., 1.) < (parameters->collisionRate * parameters->collisionRate) / 2) {
    //        sampler2.setReverb(0.5, 0.3, 0.2, 0., 0., 0.);
    //        sampler2.setDelay(-1, 0.5f, 0.7f, -1);
    //        sampler2.play(pitch, 0);
    //    }
    //}
    //-----------------------


    //triggerAtInterval(1.0, [&]() {
    //    float freq = ofMap(parameters->avgClusterVelocity, 0.0, 100.0, 0.01, 2.0);
    //    polySynth.setLFOfreq(freq);
    //    polySynth.setPitch(ofMap(parameters->clusters, 2, 20, 44, 58));
    //});

    //if (parameters->clusters > 1 && parameters->particlesInClusterRate > 0.5f) {
    //    // update the data table
    //    std::vector<float> vacValues = parameters->vacValues.get();
    //    if (dataSynth.datatable.getTableLength() != vacValues.size()) {
    //        dataSynth.datatable.setup(vacValues.size(), 1.0 * 64, true);
    //    }
    //    //dataSynth.datatable.begin();
    //    //for (int i = 0; i < vacValues.size(); ++i) {
    //    //    dataSynth.datatable.data(i, vacValues[i] * 64);
    //    //}
    //    //dataSynth.datatable.end();

    //    if (dataSynth.isPlaying == false) {
    //        dataSynth.setPitch(38);
    //        dataSynth.on(0.5f);
    //    }

    //    float cff = ofMap(vacValues[0], 0.0, 1.0, 10, 120);
    //    dataSynth.setCutOff(cff);

    //    ////triggerAtInterval(50, [&]() {
    //    ////    //float pitch = ofMap(parameters->avgClusterVelocityMagitude, 0.0, 100.0, 40.0, 60.0);
    //    ////    //dataSynth.setPitch(pitch);
    //    ////    dataSynth.on(0.6f);
    //    ////    dataSynth.setPitch(44);
    //    ////    });

    //    //triggerAtInterval(0.5, [&]() {
    //    //    float pitch = ofMap(parameters->avgClusterSize, 1.0, 20.0, 40.0, 60.0);
    //    //    dataSynth.setPitch(pitch);
    //    //    });
    //}
    //else {
    //    dataSynth.off();
    //}
}


/// discretes
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playCollisionSounds(float speedFactor) {
    int notes[10] = { -2, 0, 2, 4, 7, 1, 3, 5, 0, 4 };
    int noteShift = lastTime % 2 == 0 ? 0 : 5;
    int pitch = notes[(int)ofMap(parameters->collisionRate, 0, 1.3, noteShift, noteShift + 4)];

    bool gate = ofRandomGaussian(0., 1.) < (parameters->collisionRate * parameters->collisionRate) / 2;
    speedFactor *= ofMap(allParameters->simulationParameters.amount.get(), 100, 5000.0, 1.0, 10.0, true); //todo: get the max from somewhere sensible like the ParticlesPanel::PARTICLES_MAX

    float intervalA = 1.0 * speedFactor;
    float intervalB = 6.0 * speedFactor;

    triggerAtInterval(intervalA, [&]() {
        if (gate) {
            collisionSampler1.play(pitch, 0);
        }
        });

    triggerAtInterval(intervalB, [&]() {
        if (gate) {
            collisionSampler2.play(pitch - 6, 0);
        }
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
        collisionSampler1.add(ofToDataPath("sounds/kalimba.wav"));
        collisionSampler1.setReverb(0.5, 0.3, 1.0, 0., 0., 0.);
        collisionSampler1.setDelay(4, 0.5f, 0.7f, -1);

        collisionSampler2.add(ofToDataPath("sounds/obell-c.wav"));
        collisionSampler2.setReverb(0.7, 0.3, 10.0, 0., 0., 0.);
        collisionSampler2.setDelay(2, 0.15f, 0.6f, 0.5);
        break;
    }
}


/// background noise that responds to the VAC
/// useful for chaotic scenes: when many particles moves in crazy ways
/// or super calm scenes: many particles in screen, fewer moving (no thermostat)
/// ---------------------------------------------------------------------------------------------------
/// 
void AudioApp::setupBackgroundNoise() {
    whiteNoise.setReverb(16, 0.17, 0.8, 0.2, 0.5);
    whiteNoise.setFilter(4, 60, 0.6);
    whiteNoise.reverbDryGain.set(0.5f);
    whiteNoise.amp.set(1.0f);
}
void AudioApp::playBackgroundNoise() {
    float pitch = 45;
    float reso = 0.6f;

    // map the pitch and filter mode to the vac value
    if (parameters->vacValues.get().size() > 20) {
        pitch = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 2)], 0.0, 1.0, 30, 80);
        reso  = ofMap(parameters->vacValues.get()[int(parameters->vacValues.get().size() / 4)], 0.0, 1.0, 0.4, 0.8);
    }

    // when thermostat is on, use bandpass filter; otherwise use highpass
    int mode = allParameters->simulationParameters.applyThermostat.get() ? 4 : 0;

    float volume = ofMap(parameters->particlesInClusterRate.get(), 0.0, 1.0, 1.0, 0.3, true);

    whiteNoise.play();
    whiteNoise.setFilter(mode, pitch, 0.6f);
    whiteNoise.amp.set(volume);
}
void AudioApp::stopChaotic() {
    whiteNoise.stop();
}


/// cohesives
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playCohesive() {
    // repeat for the number of clusters
    // playback clusterSample on loop
    // modulate filter frequency and pitch based on cluster size and velocity
}
void AudioApp::stopCohesive() {
    clusterSampler1.stop();
    clusterSampler2.stop();
    clusterSampler3.stop();
}


/// empties
/// ---------------------------------------------------------------------------------------------------
void AudioApp::playEmpty() {
    collisionSampler1.stop();
    collisionSampler2.stop();
    clusterSampler1.stop();
    clusterSampler2.stop();
    clusterSampler3.stop();
    ambienceSampler.play(0.0, 0.0);
    triggerAtInterval(4.0f, [&]() {
        ambienceSampler.setReverb(0.7, 0.5, 0.3, 0., 0., 0.);
        ambienceSampler.setDelay(-1, 0.5f, 0.7f, -1);
        ambienceSampler.play(0.0, 1.0);
        ambienceSampler.fader.set(ofRandom(0.2, 0.8));
        });
}
void AudioApp::stopEmpty() {
    ambienceSampler.stop();
}



/// stop all
/// ---------------------------------------------------------------------------------------------------
void AudioApp::stopAll() {
    stopCollisionSounds();
    stopChaotic();
    stopCohesive();
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

