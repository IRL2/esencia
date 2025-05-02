#pragma once

#include "ofMain.h"
#include "../ofApp.h"
#include "../gui/GuiApp.h"
#include "ofEvents.h"
#include "particles.h"

class RenderApp : public ofBaseApp
{
    public:
        void setup();
        void update();
        void draw();

        // EVENTS
        void keyReleased(ofKeyEventArgs& e);
        void mouseMoved(int x, int y );
        void windowResized(int w, int h);
        void updateParticleBuffers();
        void renderParticlesGPU();

        


        // PARAMETERS
        // (parameters points to the mainApp's GUI. linked in main.cpp)
        RenderParameters * parameters;
        GuiApp* globalParameters;
        
        // This points directly to the simulator particles (through mainapp->simulator.particles in main.cpp)
        vector<Particle> * particles;


        Simulator* simulator; 
    
        ofEvent<glm::vec2> viewportResizeEvent; // an event to send window size updates to the simulation

    private:
        ofVbo particleVbo;
        ofShader particleShader;
        std::vector<glm::vec3> particlePositions;
        std::vector<float> particleSizes;

        ofColor particleColor;
        int particleSize;

        ofFbo fbo;
        ofFbo fboS;

        ofShader shader;
        ofShader shaderBloom;
        
};
