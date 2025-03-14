#include "RenderApp.h"


ofImage video;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

//--------------------------------------------------------------
void RenderApp::setup()
{    
    ofSetWindowTitle("esencia screen");

    ofDisableArbTex();
    ofEnableAlphaBlending();

    windowResized(ofGetWidth(), ofGetHeight());

    shader.load("shaders\\shaderBlur");
}

//--------------------------------------------------------------
void RenderApp::update()
{
    if (parameters->showVideoPreview) {
        // update the segment image from the camera
        video.setFromPixels(globalParameters->cameraParameters.previewSegment.getPixels());
        videoRectangle.x = 0;
        videoRectangle.y = ((ofGetWidth() * video.getHeight() / video.getWidth()) - ofGetHeight()) / -2;
        videoRectangle.width = ofGetWidth();
        videoRectangle.height = ofGetWidth() * video.getHeight() / video.getWidth();

        simulator->updateVideoRect(videoRectangle);
    }
}

//--------------------------------------------------------------
void RenderApp::draw()
{
	ofSetCircleResolution(10);
    ofBackground(0);

	// TODO: remove on/off toggles and use the values directly

    // draw elements
    fbo.begin();
        // solid background or trail
        if (parameters->useFaketrails) {
            ofSetColor(0, 0, 0, (int)((1-parameters->fakeTrialsVisibility) * 255));
            ofDrawRectangle(0,0,ofGetWidth(), ofGetHeight());
        }
        else {
            ofBackground(0);
        }

        // video
        if (parameters->showVideoPreview) {
            ofSetColor(parameters->videoColor, (int)(parameters->videopreviewVisibility * 255));
            video.draw(videoRectangle);
        }

        // particles
        ofSetColor(parameters->color);
        for (const auto &particle : *particles) {
            ofDrawCircle(particle.position, particle.radius);
        }

    fbo.end();

    // shader effects
    ofSetColor(255);
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

}


//--------------------------------------------------------------
void RenderApp::keyReleased(ofKeyEventArgs& e)
{
    int key = e.keycode;
    switch(key)
    {
        case 'F':
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
