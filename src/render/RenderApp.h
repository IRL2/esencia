#pragma once

#include "ofMain.h"
#include "../ofApp.h"
#include "../gui/Gui.h"
#include "ofEvents.h"
// #include <glm>

class RenderApp : public ofBaseApp
{
    public:
        void setup();
        void update();
        void draw();

        // EVENTS
        void keyPressed(int key);
        void keyReleased(int key);
        void mouseMoved(int x, int y );
        void windowResized(int w, int h);


        // PARAMETERS
        // (parameters points to the mainApp's GUI. linked in main.cpp)
        Gui::RenderParameters * parameters;
        Gui* globalParameters;
        
        // This points directly to the simulator particles (through mainapp->simulator.particles in main.cpp)
        vector<glm::vec4> * particles;

        ofEvent<glm::vec2> viewportResizeEvent; // an event to send window size updates to the simulation

    private:
        ofColor particleColor;
        int particleSize;

        ofFbo fbo;
        ofFbo fboS;

        ofShader shader;
        ofShader shaderBloom;
        
};
