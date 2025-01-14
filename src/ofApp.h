#pragma once

#include "ofMain.h"
#include "gui/GuiApp.h"
#include "camera/Camera.h"
#include "simulation/simulator.h"

class ofApp : public ofBaseApp
{
public:
    void setup();
    void update();
    void draw();
    
    void onViewportResizeEvent(glm::vec2& newSize);

    void exit() override;

    void keyReleased(ofKeyEventArgs& e);
    void windowResized(int w, int h);


    GuiApp gui;
    Camera camera;
    Simulator simulator;
};

