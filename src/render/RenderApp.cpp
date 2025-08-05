#include "RenderApp.h"

ofImage video;
ofImage particleTexture;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

ofVbo particleVbo;
ofShader particleShader;
ofShader trailShader;
ofShader videoShader;
ofShader feedbackShader; // New shader for feedback effects

// Enhanced FBO system for feedback
ofFbo videoFbo;           // Dedicated FBO for video rendering
ofFbo particlesFbo;       // Dedicated FBO for particles
ofFbo compositeFbo;       // Final composite FBO
ofFbo trailFbo;          // Existing trail FBO

// Frame history system for feedback effects
static const int MAX_FRAME_HISTORY = 4; // Store last 4 frames
std::vector<ofFbo> frameHistory;
std::vector<ofFbo> videoFrameHistory; // Separate history for video feedback
int currentFrameIndex = 0;

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

    // Load feedback shader (optional - for future effects)
    bool feedbackShaderLoaded = feedbackShader.load("shaders/feedback.vert", "shaders/feedback.frag");
    if (!feedbackShaderLoaded) {
        ofLogWarning("RenderApp::setup()") << "Feedback shaders not found - feedback effects will be disabled";
    }
    else {
        ofLogNotice("RenderApp::setup()") << "Feedback shaders loaded successfully!";
    }

    setupFBOs();

    // allocate memory for particle data
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
}

//--------------------------------------------------------------
void RenderApp::setupFBOs() {
    int w = ofGetWidth();
    int h = ofGetHeight();

    // Use GL_RGBA for better compatibility with grayscale/blurred video data
    // GL_RGBA32F_ARB can sometimes cause precision issues with intermediate alpha values
    videoFbo.allocate(w, h, GL_RGBA);
    particlesFbo.allocate(w, h, GL_RGBA);
    compositeFbo.allocate(w, h, GL_RGBA);
    trailFbo.allocate(w, h, GL_RGBA32F_ARB); // Keep high precision for trails

    // Initialize frame history for feedback effects
    frameHistory.clear();
    frameHistory.resize(MAX_FRAME_HISTORY);
    videoFrameHistory.clear();
    videoFrameHistory.resize(MAX_FRAME_HISTORY);
    
    for (int i = 0; i < MAX_FRAME_HISTORY; i++) {
        frameHistory[i].allocate(w, h, GL_RGBA);
        frameHistory[i].begin();
        ofClear(0, 0, 0, 0);
        frameHistory[i].end();
        
        videoFrameHistory[i].allocate(w, h, GL_RGBA);
        videoFrameHistory[i].begin();
        ofClear(0, 0, 0, 0);
        videoFrameHistory[i].end();
    }

    // Clear all FBOs
    clearFBO(videoFbo);
    clearFBO(particlesFbo);
    clearFBO(compositeFbo);
    clearFBO(trailFbo);

    ofLogNotice("RenderApp::setupFBOs()") << "FBO system initialized with " << MAX_FRAME_HISTORY << " frame history";
}

//--------------------------------------------------------------
void RenderApp::clearFBO(ofFbo& fbo) {
    fbo.begin();
    ofClear(0, 0, 0, 0);
    fbo.end();
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

    // 1. Render video to dedicated FBO (using previous frame for feedback)
    renderVideoPass();

    // 2. Update video frame history IMMEDIATELY after rendering current video
    // This ensures the next frame will have this frame as "previous"
    updateVideoFrameHistory();

    // 3. Render particles to dedicated FBO
    renderParticlesPass();

    // 4. Composite everything together
    renderCompositePass();

    // 5. Store current composite frame in history for other effects
    updateCompositeFrameHistory();

    // 6. Draw final result to screen
    ofSetColor(255);
    compositeFbo.draw(0, 0);

    // Draw the trail FBO to the screen using additive blending
    if (parameters->useFaketrails) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255);
        trailFbo.draw(0, 0);
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    }
}

//--------------------------------------------------------------
void RenderApp::renderVideoPass() {
    if (!parameters->showVideoPreview) return;

    videoFbo.begin();
    ofClear(0, 0, 0, 0);

    // Ensure proper blend mode for video rendering
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    renderVideoWithShader();

    videoFbo.end();
}

//--------------------------------------------------------------
void RenderApp::renderParticlesPass() {
    if (particles->empty()) return;

    particlesFbo.begin();
    ofClear(0, 0, 0, 0);

    renderParticlesGPU();

    particlesFbo.end();
}

//--------------------------------------------------------------
void RenderApp::renderCompositePass() {
    compositeFbo.begin();
    ofClear(0, 0, 0, 0);

    // Draw video layer first with proper blending for grayscale/blur
    if (parameters->showVideoPreview) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED); // Direct copy for video
        ofSetColor(255, 255, 255, 255);
        videoFbo.draw(0, 0);
    }

    // Draw particles layer with additive blending
    if (!particles->empty()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255, 255, 255, 255);
        particlesFbo.draw(0, 0);
    }

    // TODO: Add feedback effects here using previous frames
    // This is where you'll later add shader effects that use frameHistory textures

    compositeFbo.end();

    ofEnableBlendMode(OF_BLENDMODE_ALPHA); // Reset to default
}

//--------------------------------------------------------------
void RenderApp::updateCompositeFrameHistory() {
    // Get the previous index since currentFrameIndex was already incremented
    int prevIndex = (currentFrameIndex - 1 + MAX_FRAME_HISTORY) % MAX_FRAME_HISTORY;
    
    // Copy current composite to frame history
    frameHistory[prevIndex].begin();
    ofClear(0, 0, 0, 0);
    ofSetColor(255);
    compositeFbo.draw(0, 0);
    frameHistory[prevIndex].end();
}

//--------------------------------------------------------------
void RenderApp::updateFrameHistory() {
    // This method now just calls both update methods for backward compatibility
    updateCompositeFrameHistory();
    updateVideoFrameHistory();
}

//--------------------------------------------------------------
void RenderApp::updateVideoFrameHistory() {
    // Copy current video to video frame history for feedback
    if (parameters->showVideoPreview) {
        videoFrameHistory[currentFrameIndex].begin();
        ofClear(0, 0, 0, 0);
        ofSetColor(255);
        videoFbo.draw(0, 0);
        videoFrameHistory[currentFrameIndex].end();
    }
    
    // Move to next frame index (circular buffer)
    currentFrameIndex = (currentFrameIndex + 1) % MAX_FRAME_HISTORY;
}

//--------------------------------------------------------------
ofTexture& RenderApp::getPreviousFrame(int framesBack) {
    // Get a frame from N frames ago (1 = last frame, 2 = two frames ago, etc.)
    if (framesBack < 1 || framesBack > MAX_FRAME_HISTORY) {
        framesBack = 1; // Default to last frame
    }

    int index = (currentFrameIndex - framesBack + MAX_FRAME_HISTORY) % MAX_FRAME_HISTORY;
    return frameHistory[index].getTexture();
}

//--------------------------------------------------------------
ofTexture& RenderApp::getPreviousVideoFrame(int framesBack) {
    // Get a video frame from N frames ago (1 = last frame, 2 = two frames ago, etc.)
    if (framesBack < 1 || framesBack > MAX_FRAME_HISTORY) {
        framesBack = 1; // Default to last frame
    }

    int index = (currentFrameIndex - framesBack + MAX_FRAME_HISTORY) % MAX_FRAME_HISTORY;
    return videoFrameHistory[index].getTexture();
}

//--------------------------------------------------------------
void RenderApp::renderVideoWithShader() {
    // Use disabled blending to preserve grayscale/blur values exactly as they are
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    // Check if we have feedback shader for feedback effect
    if (feedbackShader.isLoaded()) {
        // Use feedback effect: blend current video with previous video frame
        
        // Bind textures
        video.getTexture().bind(0);
        getPreviousVideoFrame(1).bind(1);  // Get previous video frame
        
        feedbackShader.begin();
        
        // Set uniforms for feedback shader
        feedbackShader.setUniformTexture("currentFrame", video.getTexture(), 0);
        feedbackShader.setUniformTexture("previousFrame", getPreviousVideoFrame(1), 1);
        feedbackShader.setUniform1f("time", ofGetElapsedTimef());
        feedbackShader.setUniform2f("resolution", videoRectangle.width, videoRectangle.height);
        feedbackShader.setUniform1f("feedbackAmount", 0.9f); // Hardcoded feedback strength
        
        // Apply video color and alpha
        feedbackShader.setUniform4f("videoColor",
            parameters->videoColor.get().r / 255.0f,
            parameters->videoColor.get().g / 255.0f,
            parameters->videoColor.get().b / 255.0f,
            parameters->videopreviewVisibility);


        float auraExpansion = 100.0f; // pixels to expand in each direction
        ofRectangle expandedRect = videoRectangle;
        expandedRect.x -= auraExpansion;
        expandedRect.y -= auraExpansion;
        expandedRect.width += auraExpansion * 2;
        expandedRect.height += auraExpansion * 2;
        
        ofSetColor(255, 255, 255, 255);
        

        video.draw(videoRectangle);
        
        // Also draw a faded version in the expanded area to seed the aura
        ofSetColor(255, 255, 255, 30); // Very faint
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        video.draw(expandedRect);
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        ofSetColor(255, 255, 255, 255);

        feedbackShader.end();
        
        // Unbind textures
        getPreviousVideoFrame(1).unbind(1);
        video.getTexture().unbind(0);
    }
    else {
        // Fallback to regular video shader (no feedback)
        video.getTexture().bind(0);

        videoShader.begin();

        // Set color and alpha uniforms - existing parameter controls
        videoShader.setUniform4f("videoColor",
            parameters->videoColor.get().r / 255.0f,
            parameters->videoColor.get().g / 255.0f,
            parameters->videoColor.get().b / 255.0f,
            parameters->videopreviewVisibility);

        videoShader.setUniformTexture("tex0", video.getTexture(), 0);
        videoShader.setUniform1f("time", ofGetElapsedTimef());
        videoShader.setUniform2f("resolution", videoRectangle.width, videoRectangle.height);

        // Draw the video rectangle with shader applied
        ofSetColor(255, 255, 255, 255); // Ensure full white for proper texture rendering
        video.draw(videoRectangle);

        videoShader.end();
        video.getTexture().unbind(0);
    }
}

//--------------------------------------------------------------
void RenderApp::renderParticlesGPU() {
    if (particles->empty()) return;

    ofEnableBlendMode(OF_BLENDMODE_ADD);

    // Render main particles
    particleTexture.bind();
    particleShader.begin();

    particleShader.setUniform4f("color",
        parameters->color.get().r / 255.0f,
        parameters->color.get().g / 255.0f,
        parameters->color.get().b / 255.0f,
        parameters->color.get().a / 255.0f);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    particleVbo.draw(GL_POINTS, 0, particlePositions.size());

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

    particleShader.end();
    particleTexture.unbind();

    // Handle trails
    if (parameters->useFaketrails) {
        updateTrails();
    }

    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
}

//--------------------------------------------------------------
void RenderApp::updateTrails() {
    // Fade the trail FBO
    float fadeAmount = (1.0f - parameters->fakeTrialsVisibility) * 40.0f;
    trailFbo.begin();
    ofEnableBlendMode(OF_BLENDMODE_ALPHA);
    ofSetColor(0, 0, 0, (int)fadeAmount);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
    trailFbo.end();

    // Add new trail particles
    trailFbo.begin();
    ofEnableBlendMode(OF_BLENDMODE_ADD);

    glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
    glEnable(GL_POINT_SPRITE);
    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    particleTexture.bind();
    trailShader.begin();
    trailShader.setUniformTexture("particleTexture", particleTexture, 0);
    trailShader.setUniform4f("color",
        parameters->color.get().r / 255.0f,
        parameters->color.get().g / 255.0f,
        parameters->color.get().b / 255.0f,
        parameters->color.get().a / 255.0f);
    trailShader.setUniform1f("fadeFactor", 1.0f);

    particleVbo.draw(GL_POINTS, 0, particlePositions.size());

    trailShader.end();
    particleTexture.unbind();

    glDisable(GL_POINT_SPRITE);
    glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);

    trailFbo.end();
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
        windowResized(ofGetWidth(), ofGetHeight());
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

    // Update main FBOs
    fbo.allocate(_width, _height);
    fboS.allocate(_width, _height);

    // Reinitialize the entire FBO system
    setupFBOs();

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));
}

//--------------------------------------------------------------
void RenderApp::setupParticleBuffers() {
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
}

//--------------------------------------------------------------
void RenderApp::renderTrailsGPU() {
    // Trails are now handled within the new rendering pipeline
    // This method can be used for additional trail effects if needed
}