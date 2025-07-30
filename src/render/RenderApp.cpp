#include "RenderApp.h"

ofImage video;
ofImage particleTexture;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

ofVbo particleVbo;
ofShader particleShader;
ofShader trailShader;
ofShader videoShader;  // Add video shader
ofFbo trailFbo;
std::vector<glm::vec3> particlePositions;
std::vector<float> particleSizes;

//--------------------------------------------------------------
void RenderApp::setup()
{
    ofSetWindowTitle("esencia screen");

    ofDisableArbTex();
    ofEnableAlphaBlending();

    windowResized(ofGetWidth(), ofGetHeight());

    // Load the particle texture    
    bool textureLoaded = particleTexture.load("images/particle.png");
    if (!textureLoaded) {
        ofLogError("RenderApp::setup()") << "Failed to load particle texture!";
    }
    else
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

    // Load the trail shader
    bool trailShaderLoaded = trailShader.load("shaders/trail.vert", "shaders/trail.frag");
    if (!trailShaderLoaded) {
        ofLogError("RenderApp::setup()") << "Failed to load trail shaders!";
    }
    else {
        ofLogNotice("RenderApp::setup()") << "Trail shaders loaded successfully!";
    }

    // Load the video shader
    bool videoShaderLoaded = videoShader.load("shaders/video.vert", "shaders/video.frag");
    if (!videoShaderLoaded) {
        ofLogError("RenderApp::setup()") << "Failed to load video shaders!";
    }
    else {
        ofLogNotice("RenderApp::setup()") << "Video shaders loaded successfully!";
    }

    // Allocate the trail FBO
    trailFbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB);
    trailFbo.begin();
    ofClear(0, 0, 0, 0);
    trailFbo.end();

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

    updateParticleSystem();
}

//--------------------------------------------------------------
void RenderApp::updateParticleSystem() {
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
    ofBackground(0);

    // video with shader
    if (parameters->showVideoPreview) {
        renderVideoWithShader();
    }

    // draw particles
    renderParticlesGPU();

    fbo.end();

    ofSetColor(255);
    fbo.draw(0, 0);

    // Draw the trail FBO to the screen using additive blending to avoid black trail
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    ofSetColor(255);
    trailFbo.draw(0, 0);
    ofEnableBlendMode(OF_BLENDMODE_ALPHA); // Reset blend mode
}

//--------------------------------------------------------------
void RenderApp::renderVideoWithShader() {
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);

    // Bind video texture and start shader
    video.getTexture().bind(0);

    videoShader.begin();

    // Set color and alpha uniforms - existing parameter controls
    videoShader.setUniform4f("videoColor",
        parameters->videoColor.get().r / 255.0f,
        parameters->videoColor.get().g / 255.0f,
        parameters->videoColor.get().b / 255.0f,
        parameters->videopreviewVisibility); //  existing alpha control

    videoShader.setUniformTexture("tex0", video.getTexture(), 0);

    videoShader.setUniform1f("time", ofGetElapsedTimef());
    videoShader.setUniform2f("resolution", videoRectangle.width, videoRectangle.height);

    // Draw the video rectangle with shader applied
    video.draw(videoRectangle);

    videoShader.end();
    video.getTexture().unbind(1);
    video.getTexture().unbind(0);
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

    // Only do trail FBO and shader if useFaketrails is enabled
    if (parameters->useFaketrails) {
        // Fade the trail FBO to make trails disappear gradually
        // Use fakeTrialsVisibility to control trail length - higher values = longer trails
        float fadeAmount = (1.0f - parameters->fakeTrialsVisibility) * 40.0f; // Scale the fade amount
        trailFbo.begin();
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        ofSetColor(0, 0, 0, (int)fadeAmount); // Use calculated fade amount
        ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
        trailFbo.end();

        // Render particle trails
        trailFbo.begin();
        ofEnableBlendMode(OF_BLENDMODE_ADD);

        // Enable point sprites for trail rendering
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

        particleTexture.bind(); // Bind the particle texture, not trail texture
        trailShader.begin();
        trailShader.setUniformTexture("particleTexture", particleTexture, 0);
        trailShader.setUniform4f("color", parameters->color.get().r / 255.0f, parameters->color.get().g / 255.0f, parameters->color.get().b / 255.0f, parameters->color.get().a / 255.0f);
        trailShader.setUniform1f("fadeFactor", 1.0f); // Set to full opacity for trails

        particleVbo.draw(GL_POINTS, 0, particlePositions.size());

        trailShader.end();
        particleTexture.unbind();

        glDisable(GL_POINT_SPRITE);
        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

        trailFbo.end();
    }

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

    // Reallocate the trail FBO to match the new window size
    trailFbo.allocate(_width, _height, GL_RGBA32F_ARB);
    trailFbo.begin();
    ofClear(0, 0, 0, 0);
    trailFbo.end();

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));
}

//--------------------------------------------------------------
void RenderApp::setupParticleBuffers() {
    // Initialize particle buffers if needed
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
}

//--------------------------------------------------------------
void RenderApp::renderTrailsGPU() {
    // Simple GPU trail rendering - can be expanded if needed
    // Currently trails are rendered within renderParticlesGPU()
}