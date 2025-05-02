#include "RenderApp.h"

ofImage video;
ofImage particleTexture;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

ofVbo particleVbo;
ofShader particleShader;
std::vector<glm::vec3> particlePositions;
std::vector<float> particleSizes;

//--------------------------------------------------------------
void RenderApp::setup()
{
    ofSetWindowTitle("esencia screen");

    ofDisableArbTex();
    ofEnableAlphaBlending();

    windowResized(ofGetWidth(), ofGetHeight());

    shader.load("shaders\\shaderBlur");

	// Load the particle texture    
	bool textureLoaded = particleTexture.load("images/particle.png");
	if (!textureLoaded) {
		ofLogError("RenderApp::setup()") << "Failed to load particle texture!";
	}   else
	{
		ofLogNotice("RenderApp::setup()") << "Particle texture loaded successfully!";
	}

    bool shaderLoaded = particleShader.load("shaders/particle.vert", "shaders/particle.frag");
    if (!shaderLoaded) {
        ofLogError("RenderApp::setup()") << "Failed to load particle shaders!";
	}
	else {
		ofLogNotice("RenderApp::setup()") << "Particle shaders loaded successfully!";
	}

    // allocate memory for particle data
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
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

    updateParticleBuffers();
}

//--------------------------------------------------------------
void RenderApp::updateParticleBuffers() {
    if (particles->empty()) return;

    particlePositions.clear();
    particleSizes.clear();

    // fill with current particle data
    for (const auto& particle : *particles) {
        particlePositions.push_back(glm::vec3(particle.position.x, particle.position.y, 0));
        particleSizes.push_back(particle.radius * 2.0f);
    }

    // update VBO with new data
    particleVbo.clear();
    particleVbo.setVertexData(&particlePositions[0].x, 3, particlePositions.size(), GL_DYNAMIC_DRAW);
    particleVbo.setAttributeData(1, &particleSizes[0], 1, particleSizes.size(), GL_DYNAMIC_DRAW);
}

//--------------------------------------------------------------
void RenderApp::draw()
{
    ofSetCircleResolution(10);
    ofBackground(0);

    fbo.begin();
    // solid background or trail
    if (parameters->useFaketrails) {
        ofSetColor(0, 0, 0, (int)((1 - parameters->fakeTrialsVisibility) * 255));
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    }
    else {
        ofBackground(0);
    }

    // video
    if (parameters->showVideoPreview) {
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofSetColor(parameters->videoColor, (int)(parameters->videopreviewVisibility * 255));
        video.draw(videoRectangle);
    }

    // draw particles
    renderParticlesGPU();

    fbo.end();

    // shader effects
    ofSetColor(255);
    if (parameters->useShaders) {
        fboS.begin();
        shader.begin();
        shader.setUniform1f("blurAmnt", 3);
        shader.setUniform1f("texwidth", ofGetWidth());
        shader.setUniform1f("texheight", ofGetHeight());
        fbo.draw(0, 0);
        shader.end();
        fboS.end();

        fboS.draw(0, 0);
    }
    else {
        fbo.draw(0, 0);
    }
}

//--------------------------------------------------------------
void RenderApp::renderParticlesGPU() {
    if (particles->empty()) return;

    ofEnableBlendMode(OF_BLENDMODE_ADD);

    //shaders for point sprite rendering
    particleTexture.bind();
    particleShader.begin();

    // set uniforms
    particleShader.setUniform4f("color",
        parameters->color.get().r / 255.0f,
        parameters->color.get().g / 255.0f,
        parameters->color.get().b / 255.0f,
        parameters->color.get().a / 255.0f);

    // draw all particles in a single call
    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    particleVbo.draw(GL_POINTS, 0, particlePositions.size());

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

    particleShader.end();
    particleTexture.unbind();

    // reset blend mode
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

//--------------------------------------------------------------
void RenderApp::keyReleased(ofKeyEventArgs& e)
{
    int key = e.keycode;
    switch (key)
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
void RenderApp::mouseMoved(int x, int y) {

}

//--------------------------------------------------------------
void RenderApp::windowResized(int _width, int _height) {
    ofLogNotice("RenderApp::windowResized()") << "window resized to: " << _width << "," << _height;

    fbo.allocate(_width, _height);
    fboS.allocate(_width, _height);

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));
}