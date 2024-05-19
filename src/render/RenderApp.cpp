#include "RenderApp.h"

//--------------------------------------------------------------
void RenderApp::setup()
{
    ofSetWindowTitle("Render");

    ofSetVerticalSync(true);
    ofSetFrameRate(60);

    ofBackground(0, 0, 0, 120);

    ofDisableArbTex();

    fbo.allocate(ofGetWidth(), ofGetHeight()); // particles
    fboS.allocate(ofGetWidth(), ofGetHeight()); // shader A

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

    for (const auto &particle : *particles) {
           // particle
           ofDrawCircle(particle.position, particle.radius);
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
            fbo.allocate(ofGetWidth(), ofGetHeight());
            fboS.allocate(ofGetWidth(), ofGetHeight());
            break;
        }
        default: break;
    }
}

//--------------------------------------------------------------
void RenderApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void RenderApp::gotMessage(ofMessage msg){

}
