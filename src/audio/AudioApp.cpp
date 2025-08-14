#include "AudioApp.h"
#include "../simulation/simulator.h"

void AudioApp::setup() {
    ofLogNotice("AudioApp::setup") << "Audio application initialized";
    
    audioEnabled = true;
    lastProcessedFrame = 0;
}

void AudioApp::update() {
    if (collisionData != nullptr && 
        collisionData->frameNumber > lastProcessedFrame && 
        collisionData->collisionCount > 0) {
        
        logCollisionDetails(*collisionData);
        
        processCollisionsForAudio(*collisionData);
        
        lastProcessedFrame = collisionData->frameNumber;
    }
}

void AudioApp::logCollisionDetails(const CollisionBuffer& collisionData) {
    if (collisionData.collisionCount == 0) return;

    uint32_t actualCollisions = std::min(collisionData.collisionCount, static_cast<uint32_t>(1000));

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

void AudioApp::processCollisionsForAudio(const CollisionBuffer& collisionData) {
    if (!audioEnabled || collisionData.collisionCount == 0) return;
    
    
    // Placeholder for audio processing logic
    
}