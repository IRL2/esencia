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

    audioEngine.listDevices();
    audioEngine.setDeviceID(0); /// <----- ID of the system's output device
    audioEngine.setup(44100, 512, 3);

    ofFileDialogResult openFileResult1 = ofSystemLoadDialog("select an audio sample for clusters");

    sampler1.load(openFileResult1.getPath());

    ofFileDialogResult openFileResult2 = ofSystemLoadDialog("select an audio sample for collisions");

    sampler2.load(openFileResult2.getPath());

    sampler1.setReverb(1, 0.5, 0.2, 0.5, 3000, 0.01);
    sampler1.setDelay(0.0f, 0.0f);

    sampler2.setReverb(0.5, 0., 1.0, 0., 0., 0., 0.);
    sampler2.setDelay(2.0f, 0.5f);
    sampler2.setAHR(300., 0., 2000.);

    sampler1.out("signal") >> audioEngine.audio_out(0);
    sampler2.out("signal") >> audioEngine.audio_out(1);
    //sampler1.out("1") >> engine.audio_out(1);

}

void AudioApp::update() {
    // Process collision data
    if (collisionData != nullptr && 
        collisionData->frameNumber > lastProcessedFrame && 
        collisionData->collisionCount > 0) {
        
        logCollisionDetails(*collisionData);
        
        processCollisionsForAudio(*collisionData);
        
        lastProcessedFrame = collisionData->frameNumber;
    }
    
    // Process cluster data
    if (clusterData != nullptr && 
        clusterData->frameNumber > lastProcessedClusterFrame && 
        clusterData->clusterCount > 0) {
        
        logClusterDetails(*clusterData);
        
        processClusterStatistics(*clusterData);
        
        lastProcessedClusterFrame = clusterData->frameNumber;
    }
}

void AudioApp::logClusterDetails(const ClusterAnalysisData& clusterData) {
    if (clusterData.clusterCount == 0) return;

#ifdef this->DEBUG
    ofLogNotice("AudioApp::ClusterData") << "Frame " << clusterData.frameNumber
        << ": Processing " << clusterData.clusterCount << " clusters for audio analysis";
#endif

    // Log details for each cluster (limit to first 5 to avoid spam)
    uint32_t logLimit = std::min(clusterData.clusterCount, static_cast<uint32_t>(5));
    for (uint32_t i = 0; i < logLimit; i++) {
        const ClusterStats& cluster = clusterData.clusters[i];

#ifdef this->DEBUG
        ofLogNotice("AudioApp::ClusterDetails") << "  Cluster " << (cluster.groupIndex + 1)
            << ": " << cluster.particleCount << " particles"
            << " | Center: (" << std::fixed << std::setprecision(1) 
            << cluster.centerPosition.x << ", " << cluster.centerPosition.y << ")"
            << " | Spatial Spread: " << std::fixed << std::setprecision(2) << cluster.spatialSpread
            << " | Avg Velocity: (" << std::fixed << std::setprecision(1) 
            << cluster.averageVelocity.x << ", " << cluster.averageVelocity.y << ")"
            << " | Velocity Spread: " << std::fixed << std::setprecision(2) << cluster.velocitySpread;
#endif
    }

#ifdef this->DEBUG
    if (clusterData.clusterCount > 5) {
        ofLogNotice("AudioApp::ClusterDetails") << "  ... and " << (clusterData.clusterCount - 5) << " more clusters";
    }
#endif

}

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
    
    if (clusterData.clusterCount > 0) {
        averageClusterSize = totalParticlesInClusters / static_cast<float>(clusterData.clusterCount);
        centerOfMass /= totalParticlesInClusters;
        
        float averageSpatialSpread = totalSpatialSpread / static_cast<float>(clusterData.clusterCount);
        float averageVelocityMagnitude = totalVelocityMagnitude / static_cast<float>(clusterData.clusterCount);

#ifdef this->DEBUG
        // Log aggregate statistics for audio processing
        ofLogNotice("AudioApp::ClusterStatistics") << "Frame " << clusterData.frameNumber << " Audio Processing:"
            << " | Clusters: " << clusterData.clusterCount
            << " | Avg Size: " << std::fixed << std::setprecision(1) << averageClusterSize
            << " | Total Particles: " << static_cast<uint32_t>(totalParticlesInClusters)
            << " | Center of Mass: (" << std::fixed << std::setprecision(1) << centerOfMass.x << ", " << centerOfMass.y << ")"
            << " | Avg Spatial Spread: " << std::fixed << std::setprecision(2) << averageSpatialSpread
            << " | Avg Velocity: " << std::fixed << std::setprecision(2) << averageVelocityMagnitude;
#endif        
        
        for (uint32_t i = 0; i < clusterData.clusterCount; i++) {
            const ClusterStats& cluster = clusterData.clusters[i];
			// placeholder for audio processing logic
        }



        // 
        if (sampler1.meter_position() > 0.0f && sampler1.meter_position() < 1.0) {
            float resonanceTime = ofMap(parameters->clusters * parameters->collisionRate, 0.0, parameters->clusters, 0, 4);
            sampler1.timeControl.set(resonanceTime);
            sampler1.modAmountControl.set(parameters->collisionRate * 2.0);
            //sampler1.pitchControl.set(ofMap(parameters->clusters, 1, 5, 0.3f, 1.5f));
        }
        else {

            //sampler1.play(parameters->clusterSizeRate + 0.5f, ofClamp(parameters->clusters / 10, 0.7, 1.0));
            //float pitch = ofMap(parameters->collisionRate * parameters->clusters, 0.0, parameters->clusters, 0.2, 1.0);
            sampler1.play(1.0, 1.0);
        }

        //

        // silent hill
        //sampler1.play(clusterData.clusterCount, cluster.particleCount);

        // melodic silent hill
        //sampler1.play(clusterData.clusterCount / 10, parameters->avgClusterSize);

    }

    parameters->clusters.set((float)clusterData.clusterCount);
    parameters->clusterParticles.set((float)totalParticlesInClusters);
    parameters->avgClusterSize.set(averageClusterSize);
    parameters->avgClusterVelocity.set(totalVelocityMagnitude / static_cast<float>(clusterData.clusterCount));
    parameters->clusterSizeRate.set(totalParticlesInClusters / allParameters->simulationParameters.amount);
    parameters->collisionRate.set(parameters->collisions.get() / allParameters->simulationParameters.amount);
}

void AudioApp::logCollisionDetails(const CollisionBuffer& collisionData) {
    if (collisionData.collisionCount == 0) return;

    uint32_t actualCollisions = std::min(collisionData.collisionCount, static_cast<uint32_t>(1000));

#ifdef _DEBUG
    ofLogNotice("AudioApp::CollisionData") << "Frame " << collisionData.frameNumber
        << ": Processing " << actualCollisions << " collisions for audio";
#endif

    // Log details for each collision (limit to first 10 to avoid spam)
    uint32_t logLimit = std::min(actualCollisions, static_cast<uint32_t>(10));
    for (uint32_t i = 0; i < logLimit; i++) {
        const CollisionData& collision = collisionData.collisions[i];
        if (collision.valid) {
#ifdef this->DEBUG
            ofLogNotice("AudioApp::CollisionDetails") << "  Audio Processing Collision " << (i + 1)
                << ": Particles " << collision.particleA << " & " << collision.particleB
                << " | Distance: " << std::fixed << std::setprecision(2) << collision.distance
                << " | Velocity Magnitude: " << std::fixed << std::setprecision(2) << collision.velocityMagnitude
                << " | Pos A: (" << std::fixed << std::setprecision(1) << collision.positionA.x << ", " << collision.positionA.y << ")"
                << " | Pos B: (" << std::fixed << std::setprecision(1) << collision.positionB.x << ", " << collision.positionB.y << ")";
#endif
        }
    }

#ifdef this->DEBUG
    if (actualCollisions > 10) {
        ofLogNotice("AudioApp::CollisionDetails") << "  ... and " << (actualCollisions - 10) << " more collisions being processed for audio";
    }
#endif

    parameters->collisions.set((float)actualCollisions);
}

void AudioApp::processCollisionsForAudio(const CollisionBuffer& collisionData) {
    if (!audioEnabled || collisionData.collisionCount == 0) return;

    float triggerPhase = ofMap(parameters->collisionRate/2, 0.0, 0.5, 1.0, 0.2);
    if (sampler2.meter_position() > 0.0f && sampler2.meter_position() < triggerPhase) {
    }
    else {
        float pitch = ofMap(parameters->collisionRate / 2, 0.0, 0.5, -5.0, 5.0);
        float volum = ofMap(parameters->clusterSizeRate, 0.0, 1.0, 1.0, 0.4);
        sampler2.play(pitch, volum);
    }
}