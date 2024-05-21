#include "RenderApp.h"

//--------------------------------------------------------------
void RenderApp::setup()
{
    ofSetWindowTitle("Render");

    ofSetVerticalSync(true);
    ofSetFrameRate(60);

    ofBackground(0, 0, 0, 120);

    ofDisableArbTex();

    windowResized(ofGetWidth(), ofGetHeight()); 

    shader.load("shaderBlur");
    //shaderBloom.load("", "bloom.frag");
}
 
//--------------------------------------------------------------
void RenderApp::update(){
}   

//--------------------------------------------------------------
void RenderApp::draw()
{
    ofBackground(0);
    //ofClear(0);


    fbo.begin();

    if (parameters->useFaketrails) {
        ofSetColor(0, 0, 0, 5);
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    }
    else {
        ofBackground(0);
    }

    ofSetColor(parameters->color);

    for (int i = 0; i < particles->size(); i++) {
        ofDrawCircle(particles->at(i).x, particles->at(i).y, parameters->size);
    }
    fbo.end();


    if (parameters->useShaders) {
        fboS.begin();
        shader.begin();
        shader.setUniform1f("blurAmnt", 5);
        shader.setUniform1f("texwidth", ofGetWidth());
        shader.setUniform1f("texheight", ofGetHeight());
        fbo.draw(0,0);
        shader.end();
        fboS.end();

        //fboS.begin();
        //shader.begin();
        //shader.setUniform1f("blurAmnt", 5);
        //fbo.draw(0, 0);
        //shader.end();
        //fboS.end();

        fboS.draw(0,0);
    }
    else {
        fbo.draw(0, 0);
    }


    ofSetColor(255);
    ofDrawBitmapString((int) ofGetFrameRate(),20,20);
}

//--------------------------------------------------------------
void RenderApp::keyPressed(int key){

}

//--------------------------------------------------------------
void RenderApp::keyReleased(int key)
{
    switch(key)
    {
        case 'f':
        {
            ofToggleFullscreen();
            break;
        }
        default: break;
    }
}

//--------------------------------------------------------------
void RenderApp::mouseMoved(int x, int y ){

}

/// <summary>
/// on window resized event
/// updates the window size render parameter (simulator is listening to this from her side)
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void RenderApp::windowResized(int _width, int _height) {
    ofLogNotice("RenderApp::windowResized()") << "window resized to: " << _width << "," << _height;

    fbo.allocate(_width, _height);
    fboS.allocate(_width, _height);

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));

    //ofNotifyEvent(viewportResizeEvent, newSize, this); // not used now
}
