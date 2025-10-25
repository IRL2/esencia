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


    /// sound setup
    audioEngine.listDevices();
    audioEngine.setDeviceID(0); // todo: add control to change this from gui (currently uses the system's default interface)
    audioEngine.setup(44100, 512, 3);


    // load sound files
    //ofFileDialogResult openFileResult1 = ofSystemLoadDialog("select an audio sample for clusters");
    //sampler1.load(openFileResult1.getPath());
    sampler1.add(ofToDataPath("sounds/omissed-track.wav"));

    //ofFileDialogResult openFileResult2 = ofSystemLoadDialog("select an audio sample for collisions");
    //sampler2.load(openFileResult2.getPath());
    //sampler2.load(ofToDataPath("sounds/obell-c2.wav"), 0);
    sampler2.add(ofToDataPath("sounds/kalimba-soft2.wav"));
    //sampler2.load(ofToDataPath("sounds/pianofelt12.wav"), 2);


    // config sampler player
    sampler1.setReverb(1, 0.5, 0.2, 0.5, 3000, 0.01);
    sampler1.setDelay(0.0f, 0.0f);

    sampler2.setReverb(0.5, 0.3, 0.2, 0., 1000, 0., 0.);
    sampler2.setDelay(0.5f, 0.7f);
    sampler2.setAHR(100., 100., 2000.);
    //sampler2.play(1.0, 1.0);

    // configure synth
    polySynth.setup(8);
    polySynth.on(1.0f);
    polySynth.setPitch(46);

    dataSynth.datatable.setup(100, 64, true);
    dataSynth.setup();
    dataSynth.setLFOfreq(0);
    dataSynth.setPitch(44);

    // connect all modules to a master gain then to the audio engine
    sampler1.fader.ch(0) >> masterAmp.ch(0);
    sampler2.fader.ch(0) >> masterAmp.ch(0);
    polySynth.ch(0) >> masterAmp.ch(0);
    polySynth.ch(1) >> masterAmp.ch(1);
    dataSynth.ch(0) >> masterAmp.ch(0);
    dataSynth.ch(1) >> masterAmp.ch(1);
    masterAmp.ch(0) >> audioEngine.audio_out(0);
    masterAmp.ch(1) >> audioEngine.audio_out(1);

    masterAmp >> mainScope >> audioEngine.blackhole();

    // initial volumes
    parameters->masterVolume.set(0.8);
    parameters->polysynthVolume.set(0.6);
    parameters->sampler1playerVolume.set(0.7);
    parameters->sampler2playerVolume.set(0.7);
    parameters->datasynthVolume.set(0.5);
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

}


void AudioApp::draw() {
    ofPushStyle();
    ofSetColor(255, 255, 255, 100);
    mainScope.draw(-1, -1, ofGetWidth()+2, ofGetHeight()+2);
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
    parameters->particlesInClusterRate.set(totalParticlesInClusters / allParameters->simulationParameters.amount);
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


    // update volumes from parameters
    masterAmp.set(parameters->masterVolume);
    sampler1.fader.set(ofMap(parameters->sampler1playerVolume, 0.0, 1.0, -48, 12));
    sampler2.fader.set(ofMap(parameters->sampler2playerVolume, 0.0, 1.0, -48, 12));
    polySynth.gain.set(ofMap(parameters->polysynthVolume, 0.0, 1.0, -80, -12));
    dataSynth.gain.set(ofMap(parameters->datasynthVolume, 0.0, 1.0, -48, 12));


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




    if (sampler1.meter_position() > 0.0f && sampler1.meter_position() < 1.0) {
        //float resonanceTime = ofMap(parameters->clusters * parameters->collisionRate, 0.0, parameters->clusters, 0, 4);
        //sampler1.timeControl.set(resonanceTime);
        sampler1.modAmountControl.set(parameters->particlesInClusterRate * 2.0);
        //sampler1.pitchControl.set(ofMap(parameters->clusters, 1, 5, 0.3f, 1.5f));
    }
    else {
        //sampler1.play(parameters->particlesInClusterRate + 0.5f, ofClamp(parameters->clusters / 10, 0.7, 1.0));
        //float pitch = ofMap(parameters->collisionRate * parameters->clusters, 0.0, parameters->clusters, 0.2, 1.0);
        sampler1.play(1.0, 2.0); // prev
    }

    // silent hill theme
    //sampler1.play(clusterData.clusterCount, cluster.particleCount);
    // melodic silent hill
    //sampler1.play(clusterData.clusterCount / 10, parameters->avgClusterSize);

    size_t samplerIndex = sampler2.currentSampleIndex;
    triggerAtInterval(2.0, [&]() {
        samplerIndex = (samplerIndex + 1) % sampler2.getNumSamples();
        sampler2.switchSampleIndex(samplerIndex);
        //ofLog() << "switching to sample #" << samplerIndex;
        });

    // selecting "notes" (pitch) from two scales, alternating every second
    // this actually works well
    int notesA[5] = { -2, 0, 2, 4, 7 };
    int notesB[5] = { 1, 3, 5, 0, 4 };
    int notes[] = { 0,0,0,0,0 };
    std::copy(notesA, notesA + 5, notes);
    if (lastTime % 2 == 0) {
        std::copy(notesB, notesB + 5, notes);
    }
    int pitch = notes[(int)ofMap(parameters->collisionRate, 0, 1.3, 0, 4)];


    //float pitch = ceilf(ofMap(parameters->collisionRate, 0.0, 1.5, -5.0, 5.0));
    //float volum = ofMap(parameters->particlesInClusterRate, 0.0, 1.0, 1.0, 0.4);
    //float vol = ofClamp(parameters->collisionRate * 1.5, -0.3, 1.0); // original
    //float vol = ofMap(parameters->clusters, 1, 8, 20, -50);
    float vol = ofMap(parameters->sampler1playerVolume, 0.0, 1.0, -30, 5);

    //parameters->samplerplayerVolume.set(vol);


    if (checkInterval(1.0)) {
        if (ofRandomGaussian(0., 1.) < (parameters->collisionRate * parameters->collisionRate) / 2) {
            sampler2.setReverb(0.5, 0.3, 0.2, 0., 1000, 0., 0.);
            sampler2.setDelay(0.5f, 0.7f);
            sampler2.play(pitch, 0);
        }
    }

    //triggerAtInterval(1.0, [&]() {
    //    float freq = ofMap(parameters->avgClusterVelocity, 0.0, 100.0, 0.01, 2.0);
    //    polySynth.setLFOfreq(freq);
    //    polySynth.setPitch(ofMap(parameters->clusters, 2, 20, 44, 58));
    //});




    if (parameters->clusters > 1 && parameters->particlesInClusterRate > 0.5f) {
        // update the data table
        std::vector<float> vacValues = parameters->vacValues.get();
        if (dataSynth.datatable.getTableLength() != vacValues.size()) {
            dataSynth.datatable.setup(vacValues.size(), 1.0 * 64, true);
        }
        //dataSynth.datatable.begin();
        //for (int i = 0; i < vacValues.size(); ++i) {
        //    dataSynth.datatable.data(i, vacValues[i] * 64);
        //}
        //dataSynth.datatable.end();

        if (dataSynth.isPlaying == false) {
            dataSynth.setPitch(38);
            dataSynth.on(0.5f);
        }

        float cff = ofMap(vacValues[0], 0.0, 1.0, 10, 120);
        dataSynth.setCutOff(cff);

        ////triggerAtInterval(50, [&]() {
        ////    //float pitch = ofMap(parameters->avgClusterVelocityMagitude, 0.0, 100.0, 40.0, 60.0);
        ////    //dataSynth.setPitch(pitch);
        ////    dataSynth.on(0.6f);
        ////    dataSynth.setPitch(44);
        ////    });

        //triggerAtInterval(0.5, [&]() {
        //    float pitch = ofMap(parameters->avgClusterSize, 1.0, 20.0, 40.0, 60.0);
        //    dataSynth.setPitch(pitch);
        //    });
    }
    else {
        dataSynth.off();
    }
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

