#pragma once

#include "ofMain.h"
#include <GL/glew.h>
#include "../gui/GuiApp.h"
//#include "ofxOpenCv.h" // TODO: research on using a different datastructure to pass the frame segment and avoid loading opencv here
#include "particles.h"
#include <unordered_set>

struct CollisionData {
    uint32_t particleA;
    uint32_t particleB;
    glm::vec2 positionA;
    glm::vec2 positionB;
    float distance;
    float velocityMagnitude;
    uint32_t valid;
    uint32_t padding; // for alignment
};

struct CollisionBuffer {
    uint32_t collisionCount;
    uint32_t maxCollisions;
    uint32_t frameNumber;
    uint32_t padding;
    std::vector<CollisionData> collisions;
};

// Cluster analysis structures
struct ClusterStats {
    uint32_t groupIndex;           // Unique cluster ID
    uint32_t particleCount;        // Number of particles in cluster
    glm::vec2 centerPosition;      // Spatial center of cluster
    float spatialSpread;           // Standard deviation of particle positions
    glm::vec2 averageVelocity;     // Average velocity of cluster
    float velocitySpread;          // Standard deviation of velocities
    uint32_t frameNumber;          // Frame when this cluster was detected
};

struct ClusterAnalysisData {
    uint32_t clusterCount;         // Number of clusters found
    uint32_t frameNumber;          // Current frame number
    uint32_t minClusterSize;       // Minimum particles to consider a cluster
    uint32_t maxClusters;          // Maximum number of clusters we can store
    std::vector<ClusterStats> clusters;
};

// VAC (Velocity Autocorrelation Function) structures
struct VACData {
    std::vector<float> vacValues;           // VAC(t) values for each time lag
    std::vector<float> timePoints;          // Time points corresponding to each VAC value
    uint32_t maxTimeLags;                   // Maximum number of time lags to calculate
    uint32_t currentFrame;                  // Current frame number for data collection
    bool isEnabled;                         // Whether VAC calculation is enabled
    uint32_t lastCalculationFrame;          // Track when VAC was last calculated
    
    VACData() : maxTimeLags(60), currentFrame(0), isEnabled(true), lastCalculationFrame(0) {
        vacValues.resize(maxTimeLags, 0.0f);
        timePoints.resize(maxTimeLags);
        for (uint32_t i = 0; i < maxTimeLags; i++) {
            timePoints[i] = static_cast<float>(i) * 0.01f;
        }
    }
};

class Simulator {
public:
    void setup(SimulationParameters* params, GuiApp* globalParams);
    void update();
    void updateWorldSize(int _width, int _height);
    void recieveFrame(ofxCvGrayscaleImage& frame);
    void updateVideoRect(const ofRectangle& rect);

    void keyReleased(ofKeyEventArgs& e);

    ParticleSystem particles;
    
    CollisionBuffer collisionData;
    ClusterAnalysisData clusterData;
    VACData vacData;
    
    // VAC control methods
    bool isVACEnabled() const { return enableVACCalculation; }
    void setVACEnabled(bool enabled) { enableVACCalculation = enabled; }
    uint32_t getMaxVelocityFrames() const { return maxVelocityFrames; }
    
    // Collision data access method
    const CollisionBuffer& getCollisionData() const { return collisionData; }
    bool isCollisionLoggingEnabled() const { return parameters->enableCollisionLogging; }
    void setCollisionLoggingEnabled(bool enabled) { parameters->enableCollisionLogging = enabled; }
    
    // Cluster analysis control methods
    bool isClusterAnalysisEnabled() const { return enableClusterAnalysis; }
    void setClusterAnalysisEnabled(bool enabled) { enableClusterAnalysis = enabled; }
    float getClusterConnectionDistance() const { return clusterConnectionDistance; }
    void setClusterConnectionDistance(float distance) { clusterConnectionDistance = distance; }

    GLint deltaTimeLocation;
    GLint worldSizeLocation;
    GLint targetTemperatureLocation;
    GLint couplingLocation;
    GLint applyThermostatLocation;
    GLint depthFieldScaleLocation;
    GLint videoOffsetLocation;
    GLint videoScaleLocation;
    GLint sourceSizeLocation;
    GLint ljEpsilonLocation;
    GLint ljSigmaLocation;
    GLint ljCutoffLocation;
    GLint maxForceLocation;
    GLint depthFieldLocation;
    GLint enableCollisionLoggingLocation;

    static const size_t MAX_COLLISIONS_PER_FRAME = 1024;
    static const size_t MAX_CLUSTERS_PER_FRAME = 10;
    static const uint32_t MIN_CLUSTER_SIZE = 10;

private:
    void setupComputeShader();
    void updateParticlesOnGPU();
    void updateDepthFieldTexture();
    void setupCollisionBuffer();
    void readCollisionData();
    
    // Cluster analysis methods
    void setupClusterAnalysis();
    void analyzeParticleClusters();
    void performUnionFind(std::vector<uint32_t>& parent, std::vector<uint32_t>& rank);
    uint32_t findRoot(std::vector<uint32_t>& parent, uint32_t particle);
    void unionSets(std::vector<uint32_t>& parent, std::vector<uint32_t>& rank, uint32_t a, uint32_t b);
    void calculateClusterStatistics(const std::vector<uint32_t>& parent, const std::vector<std::unordered_set<uint32_t>>& clusterMembers);

    // VAC (Velocity Autocorrelation Function) methods
    void setupVACAnalysis();
    void calculateVAC();
    void storeVelocityFrame();

    // listeners
    void onRenderwindowResize(glm::vec2& worldSize);
    void onGUIChangeAmmount(float& value);
    void onGUIChangeRadius(int& value);
    void onApplyThermostatChanged(bool& value);
    void onTemperatureChanged(float& value);
    void onCouplingChanged(float& value);
    void onDepthFieldScaleChanged(float& value);
    void onCollisionLoggingChanged(bool& value);
    void applyBerendsenThermostat();

    GLuint ssboParticles;
    GLuint ssboCollisions;
    GLuint computeShaderProgram;
    GLuint depthFieldTexture;

    bool applyThermostat = true;
    float targetTemperature = 2000.0;
    float coupling = 0.5;
    float depthFieldScale = -100000.0f;
    bool hasDepthField = false;
    float ljEpsilon = 10.0f;    // Lennard-Jones well depth
    float ljCutoff = 150.0f;    // Interaction cutoff
    float maxForce = 10000.0f;  // Force clamping

    ofxCvGrayscaleImage* currentDepthField;

    ofRectangle videoRect = ofRectangle(0, 0, -45, -45);
    float videoScaleX = 1.4;
    float videoScaleY = 1.4f;
    int sourceWidth = 640;  // camera resolution width
    int sourceHeight = 576; // camera resolution height

    int width = 640;
    int height = 576;

    SimulationParameters* parameters;
    GuiApp* globalParameters;

    const float INV255 = 1.0f / 255.0f;

    // Internal collision buffer for GPU operations
    CollisionBuffer collisionBuffer;
    uint32_t currentFrameNumber = 0;
    
    // Cluster analysis settings
    bool enableClusterAnalysis = true;
    float clusterConnectionDistance = 50.0f;

    // VAC calculation settings and data storage
    std::vector<std::vector<glm::vec2>> velocityHistory; 
    uint32_t maxVelocityFrames = 512;
    uint32_t vacCalculationInterval = 5;                 
    bool enableVACCalculation = true;

    // Normalization functions
    glm::vec2 normalizePosition(const glm::vec2& position);
    float normalizeDistance(float distance);
};