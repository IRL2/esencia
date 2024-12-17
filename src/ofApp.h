#pragma once

#include "ofMain.h"
#include "gui/Gui.h"
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

    void keyReleased(int key);
    void windowResized(int w, int h);


    GuiApp gui;
    Camera camera;
    Simulator simulator;
};

