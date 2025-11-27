#pragma once

#include "ofMain.h"
#include "../ofApp.h"
#include "../gui/GuiApp.h"
#include "ofEvents.h"
#include "particles.h"
#include <deque>

class RenderApp : public ofBaseApp
{
    public:
        void setup();
        void update();
        void draw();

        // EVENTS
        void keyReleased(ofKeyEventArgs& e);
        void onWindowResized(ofResizeEventArgs &args);
        
        //  rendering methods
        void updateParticleSystem();
        void renderParticlesGPU();
        void renderTrailsGPU();
        void setupParticleBuffers();
		void renderVideoWithShader();
		void renderVideoStandard();

        void setupWindowSize(int width, int height);
        void setupFBOs(int _width, int _height);
        void clearFBO(ofFbo& fbo);
        void renderVideoPass();
        void renderParticlesPass();
        void renderCompositePass();
        void updateFrameHistory();
        void updateVideoFrameHistory();
        void updateCompositeFrameHistory();
        void updateTrails();
        ofTexture& getPreviousVideoFrame(int framesBack = 1);

        // PARAMETERS
        // (parameters points to the mainApp's GUI. linked in main.cpp)
        RenderParameters * parameters;
        GuiApp* globalParameters;
        
        // This points directly to the simulator particles (through mainapp->simulator.particles in main.cpp)
        vector<Particle> * particles;

        Simulator* simulator; 
    
        ofEvent<glm::vec2> viewportResizeEvent; // an event to send window size updates to the simulation

        // to handle local configuration values, out of the presets
        ofxGui fakeGui;
        ofxGuiPanel* localSettings;
        ofParameterGroup& localSettingsValues = ofParameterGroup();
        ofParameter<int> windowPositionX;
        ofParameter<int> windowPositionY;
        ofParameter<bool> startFullscreen;
        ofParameter<bool> displayCameraWarning;

    private:

        ofVbo particleVbo;
        ofShader particleShader;
        ofShader trailShader;
        
        std::vector<glm::vec3> particlePositions;
        std::vector<float> particleSizes;

        ofColor particleColor;
        int particleSize;

        ofFbo fbo;
        ofFbo fboS;
        ofFbo trailFbo; 
        // New FBO system for feedback
        ofFbo videoFbo; 
        ofFbo particlesFbo; 
        ofFbo compositeFbo; 

        static const int MAX_FRAME_HISTORY = 4;
        std::vector<ofFbo> frameHistory;
        int currentFrameIndex;

        ofShader shader;
        ofShader shaderBloom;
};
