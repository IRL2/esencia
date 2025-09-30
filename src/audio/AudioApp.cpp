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

    ofFileDialogResult openFileResult = ofSystemLoadDialog("select an audio sample");

    sampler1.load(openFileResult.getPath());

    sampler1.out("signal") >> audioEngine.audio_out(0);
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
            
            if (sampler1.meter_position() > 0.0f && sampler1.meter_position() < 0.99f) {

            }

            sampler1.play(clusterData.clusterCount, cluster.particleCount);


			// placeholder for audio processing logic
        }
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

//#ifdef _DEBUG
    ofLogNotice("AudioApp::CollisionData") << "Frame " << collisionData.frameNumber
        << ": Processing " << actualCollisions << " collisions for audio";
//#endif

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
    //if (!audioEnabled || collisionData.collisionCount == 0) return;
    
    //ofLog() << collisionData.collisions.size();
    //parameters->collisions.set((float)collisionData.collisions.size());
    // Placeholder for audio processing logic
}