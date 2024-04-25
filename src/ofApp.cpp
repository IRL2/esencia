#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

    ofSetWindowTitle("GUI");

    ofSetVerticalSync(false);
    ofSetFrameRate(60);

    gui.setup();
    
    ofBackground(0);

    // to-do: pass camera parameters though setup
    camera.setup();
    camera.linkGuiParams(&gui.cameraParameters);

    simulator.setup(&gui.simulationParameters);
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