#pragma once

#include "ofMain.h"
#include <vector>
#include <iomanip>

// defined in simulator.h
struct CollisionData;
struct CollisionBuffer;

class AudioApp {
public:
    void setup();
    void update();
    

    void logCollisionDetails(const CollisionBuffer& collisionData);
    void processCollisionsForAudio(const CollisionBuffer& collisionData);

    CollisionBuffer* collisionData = nullptr;

private:

    uint32_t lastProcessedFrame = 0;
    bool audioEnabled = true;
};
