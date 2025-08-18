#pragma once

#include "ofMain.h"
#include <vector>
#include <iomanip>

// Forward declarations
struct CollisionData;
struct CollisionBuffer;
struct ClusterStats;
struct ClusterAnalysisData;

class AudioApp {
public:
    void setup();
    void update();
    

    void logCollisionDetails(const CollisionBuffer& collisionData);
    void processCollisionsForAudio(const CollisionBuffer& collisionData);
    
    // New cluster analysis methods
    void logClusterDetails(const ClusterAnalysisData& clusterData);
    void processClusterStatistics(const ClusterAnalysisData& clusterData);

    CollisionBuffer* collisionData = nullptr;
    ClusterAnalysisData* clusterData = nullptr;

private:

    uint32_t lastProcessedFrame = 0;
    uint32_t lastProcessedClusterFrame = 0;
    bool audioEnabled = true;
    bool clusterAnalysisEnabled = true;
};
