#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetWindowTitle("esencia");

    ofSetVerticalSync(false);
    ofSetFrameRate(30);

    gui.setup();
    
    audioApp.setup();

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
    
    audioApp.update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // camera.draw();
    gui.draw();
}

void ofApp::exit() {
    cout << "\nturning off the camera";
    camera.exit();
    ofSleepMillis(1000);
    cout << "\nBYE!";
    ofBaseApp::exit();
}


void ofApp::keyReleased(ofKeyEventArgs& e) {
    gui.keyReleased(e);
    camera.keyReleased(e);
	simulator.keyReleased(e);
}


void ofApp::windowResized(int w, int h)
{
    gui.windowResized(w, h);
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
