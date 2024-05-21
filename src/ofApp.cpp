#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetWindowTitle("GUI");

    ofSetVerticalSync(false);
    ofSetFrameRate(60);

    gui.setup();
    
    ofBackground(0);

    // to-do: pass camera parameters though setup
    camera.setup(&gui.cameraParameters);

    simulator.setup(&gui.simulationParameters, &gui);
}
//--------------------------------------------------------------
void ofApp::update(){
    gui.update();

    camera.update();

    simulator.recieveFrame(camera.segment);

    simulator.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    camera.draw();
    gui.draw();
}

void ofApp::exit() {
    cout << "\nturning off the camera";
    camera.exit();
    ofSleepMillis(1000);
    cout << "\nBYE!";
    ofBaseApp::exit();
}


void ofApp::keyReleased(int key)
{
    switch (key)
    {

    case 'm':
        //camera.saveMeshFrame();
        break;

    case 'p':
        //camera.savePixelsFrame();
        break;

    case 's':
        //camera.startBackgroundReferenceSampling();
        break;
    case 'd':
        //camera.clearBackgroundReference();
        break;
    case '1':
        camera.startBackgroundReferenceSampling();
        break;

    default:
        break;

    }



}

/// <summary>
/// NOT USED RIGHT NOW
/// Called on the render window resize event
/// This will pass the new size to the simulator parameters (and the simulator has a listener on the parameters update)
/// </summary>
/// <param name="newSize"></param>
void ofApp::onViewportResizeEvent(glm::vec2& newSize) {
    ofLogNotice("ofApp::onViewportResizeEvent()") << "Recieving window resize message: " << newSize;
    simulator.updateWorldSize(newSize.x, newSize.y);
}

