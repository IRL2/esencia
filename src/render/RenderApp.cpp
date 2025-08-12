#include "RenderApp.h"

ofImage video;
ofImage particleTexture;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

ofVbo particleVbo;
ofShader particleShader;
ofShader trailShader;
ofShader videoShader;
ofShader feedbackShader; 


ofFbo videoFbo;
ofFbo particlesFbo;
ofFbo compositeFbo;
ofFbo trailFbo;


static const int MAX_FRAME_HISTORY = 4;
std::vector<ofFbo> frameHistory;
std::vector<ofFbo> videoFrameHistory;
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

    bool feedbackShaderLoaded = feedbackShader.load("shaders/feedback.vert", "shaders/feedback.frag");
    if (!feedbackShaderLoaded) {
        ofLogWarning("RenderApp::setup()") << "Feedback shaders not found - feedback effects will be disabled";
    }
    else {
        ofLogNotice("RenderApp::setup()") << "Feedback shaders loaded successfully!";
    }


    bool warpUpdateLoaded = warpUpdateShader.load("shaders/warp_update.vert", "shaders/warp_update.frag");
    bool warpApplyLoaded = warpApplyShader.load("shaders/warp_apply.vert", "shaders/warp_apply.frag");
    
    if (!warpUpdateLoaded || !warpApplyLoaded) {
        ofLogWarning("RenderApp::setup()") << "Two-pass warp shaders failed to load";
    }
    
    setupFBOs();
    setupWarpFBOs(); 

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
    trailFbo.allocate(w, h, GL_RGBA32F_ARB);

    // Initialize frame history 
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
void RenderApp::setupWarpFBOs() {
    int w = ofGetWidth();
    int h = ofGetHeight();
    
    displacementFbo.allocate(w, h, GL_RG16F);
    previousDisplacementFbo.allocate(w, h, GL_RG16F);
    
    displacementFbo.begin();
    ofClear(0, 0, 0, 0);
    displacementFbo.end();
    
    previousDisplacementFbo.begin();
    ofClear(0, 0, 0, 0);
    previousDisplacementFbo.end();
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

    // 1. Render video to dedicated FBO
    renderVideoPass();

    // 2. Update video frame history
    updateVideoFrameHistory();

    // 3. Render particles to dedicated FBO
    renderParticlesPass();

    // 4. Composite everything together
    renderCompositePass();

    // 5. Store current composite frame 
    updateCompositeFrameHistory();

    // 6. Draw final result
    ofSetColor(255);
    compositeFbo.draw(0, 0);

    // Draw the trail FBO
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

    if (parameters->showVideoPreview) {
        ofEnableBlendMode(OF_BLENDMODE_DISABLED);
        ofSetColor(255, 255, 255, 255);
        videoFbo.draw(0, 0);
    }

    if (!particles->empty()) {
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        ofSetColor(255, 255, 255, 255);
        particlesFbo.draw(0, 0);
    }


    compositeFbo.end();

    ofEnableBlendMode(OF_BLENDMODE_ALPHA); 
}

//--------------------------------------------------------------
void RenderApp::updateCompositeFrameHistory() {
    // Stores the final composite frame (video + particles combined)
    int prevIndex = (currentFrameIndex - 1 + MAX_FRAME_HISTORY) % MAX_FRAME_HISTORY;
    
    frameHistory[prevIndex].begin();
    ofClear(0, 0, 0, 0);
    ofSetColor(255);
    compositeFbo.draw(0, 0);
    frameHistory[prevIndex].end();
}

//--------------------------------------------------------------
void RenderApp::updateFrameHistory() {
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
ofTexture& RenderApp::getPreviousVideoFrame(int framesBack) {
    // get a video frame from N frames ago (1 = last frame, 2 = two frames ago, etc.)
    if (framesBack < 1 || framesBack > MAX_FRAME_HISTORY) {
        framesBack = 1; // Default to last frame
    }

    int index = (currentFrameIndex - framesBack + MAX_FRAME_HISTORY) % MAX_FRAME_HISTORY;
    return videoFrameHistory[index].getTexture();
}

//--------------------------------------------------------------
void RenderApp::renderVideoWithShader() {
    // use disabled blending to preserve grayscale/blur values exactly as they are
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    if (parameters->useWarpEffect && feedbackShader.isLoaded()) {
        // === WARP EFFECT RENDERING PATH ===
        renderVideoWithFeedback();
    }
    else {
        // === STANDARD RENDERING PATH (PASS-THROUGH) ===
        renderVideoStandard();
    }
}

//--------------------------------------------------------------
void RenderApp::renderVideoWithFeedback() {
    //single pass warping.displacement calculation and texture sampling occur in the same shader.
    //uses the current frame and previous frame as input.
    //calculates displacement on the fly and immediately applies it.

    //to do: remove....
    if (parameters->useTwoPassWarp && warpUpdateShader.isLoaded() && warpApplyShader.isLoaded()) {
        renderVideoWithWarpTwoPass();
    } else {
        video.getTexture().bind(0);
        getPreviousVideoFrame(1).bind(1);
        
        feedbackShader.begin();
        

        feedbackShader.setUniformTexture("currentFrame", video.getTexture(), 0);
        feedbackShader.setUniformTexture("previousFrame", getPreviousVideoFrame(1), 1);
        feedbackShader.setUniform1f("time", ofGetElapsedTimef());
        feedbackShader.setUniform2f("resolution", ofGetWidth(), ofGetHeight());
        feedbackShader.setUniform1f("feedbackAmount", 1.0f);
        
        // Pass video rectangle info to shader for proper coordinate mapping
        feedbackShader.setUniform4f("videoRect", 
            videoRectangle.x, videoRectangle.y, 
            videoRectangle.width, videoRectangle.height);
        
        feedbackShader.setUniform4f("videoColor",
            parameters->videoColor.get().r / 255.0f,
            parameters->videoColor.get().g / 255.0f,
            parameters->videoColor.get().b / 255.0f,
            parameters->videopreviewVisibility);

        feedbackShader.setUniform1f("warpVariance", parameters->warpVariance);
        feedbackShader.setUniform1f("warpPropagation", parameters->warpPropagation);
        feedbackShader.setUniform1f("warpPropagationPersistence", parameters->warpPropagationPersistence);
        feedbackShader.setUniform1f("warpSpreadX", parameters->warpSpreadX);
        feedbackShader.setUniform1f("warpSpreadY", parameters->warpSpreadY);
        feedbackShader.setUniform1f("warpDetail", parameters->warpDetail);
        feedbackShader.setUniform1f("warpBrightPassThreshold", parameters->warpBrightPassThreshold);

        ofSetColor(255, 255, 255, 255);
        video.draw(videoRectangle);

        feedbackShader.end();
        
        getPreviousVideoFrame(1).unbind(1);
        video.getTexture().unbind(0);
    }
}

//--------------------------------------------------------------
void RenderApp::renderVideoWithWarpTwoPass() {

    //How it works (or should lol)
    //Pass 1 (warp_update.vert / frag) for calculating and storing displacement vectors.
    //takes current video frame and previous displacement as input,
    //outputs displacement data to a dedicated FBO,
    //focuses purely on displacement computation and propagation

    //Pass 2 (warp_apply.vert / frag) is for applying the precalculated displacement.
    // takes the original video frame and displacement texture as input,
    // applies warping using the displacement data from pass 1,
    // outputs the final warped video frame

    if (!video.isAllocated() || video.getWidth() == 0 || video.getHeight() == 0) {
        ofLogWarning("RenderApp::renderVideoWithWarpTwoPass") << "Video texture not properly allocated";
        renderVideoStandard(); // Fallback to standard rendering
        return;
    }

    // === PASS 1: Update Displacement ===
    displacementFbo.begin();
    ofClear(0, 0, 0, 0);
    
    ofViewport(0, 0, displacementFbo.getWidth(), displacementFbo.getHeight());
    
    // Bind textures for first pass
    video.getTexture().bind(0);
    previousDisplacementFbo.getTexture().bind(1);
    
    warpUpdateShader.begin();
    warpUpdateShader.setUniformTexture("externalInput", video.getTexture(), 0);
    warpUpdateShader.setUniformTexture("currentDisplacement", previousDisplacementFbo.getTexture(), 1);
    
    warpUpdateShader.setUniform1f("warpVariance", parameters->warpVariance);
    warpUpdateShader.setUniform1f("warpPropagation", parameters->warpPropagation);
    warpUpdateShader.setUniform1f("warpPropagationPersistence", parameters->warpPropagationPersistence);
    warpUpdateShader.setUniform1f("warpSpreadX", parameters->warpSpreadX);
    warpUpdateShader.setUniform1f("warpSpreadY", parameters->warpSpreadY);
    warpUpdateShader.setUniform1f("warpDetail", parameters->warpDetail);
    
    ofMesh fullscreenQuad;
    fullscreenQuad.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    
    // Add vertices with proper texture coordinates for fullscreen
    fullscreenQuad.addVertex(ofVec3f(-1, -1, 0));
    fullscreenQuad.addTexCoord(ofVec2f(0, 0));
    
    fullscreenQuad.addVertex(ofVec3f(1, -1, 0));
    fullscreenQuad.addTexCoord(ofVec2f(1, 0));
    
    fullscreenQuad.addVertex(ofVec3f(-1, 1, 0));
    fullscreenQuad.addTexCoord(ofVec2f(0, 1));
    
    fullscreenQuad.addVertex(ofVec3f(1, 1, 0));
    fullscreenQuad.addTexCoord(ofVec2f(1, 1));
    
    // Disable depth testing and set proper matrices
    ofPushMatrix();
    ofSetMatrixMode(OF_MATRIX_PROJECTION);
    ofPushMatrix();
    ofLoadIdentityMatrix();
    ofSetMatrixMode(OF_MATRIX_MODELVIEW);
    ofPushMatrix();
    ofLoadIdentityMatrix();
    
    fullscreenQuad.draw();
    
    // Restore matrices
    ofPopMatrix();
    ofSetMatrixMode(OF_MATRIX_PROJECTION);
    ofPopMatrix();
    ofSetMatrixMode(OF_MATRIX_MODELVIEW);
    ofPopMatrix();
    
    warpUpdateShader.end();
    
    previousDisplacementFbo.getTexture().unbind(1);
    video.getTexture().unbind(0);
    displacementFbo.end();
    
    // Reset viewport
    ofViewport(0, 0, ofGetWidth(), ofGetHeight());
    
    // === PASS 2: Apply Displacement ===
    // Bind textures for second pass
    video.getTexture().bind(0);
    displacementFbo.getTexture().bind(1);
    
    warpApplyShader.begin();
    warpApplyShader.setUniformTexture("sourceTexture", video.getTexture(), 0);
    warpApplyShader.setUniformTexture("displacementTexture", displacementFbo.getTexture(), 1);
    
    warpApplyShader.setUniform4f("videoColor",
        parameters->videoColor.get().r / 255.0f,
        parameters->videoColor.get().g / 255.0f,
        parameters->videoColor.get().b / 255.0f,
        parameters->videopreviewVisibility);
    
    warpApplyShader.setUniform1f("warpBrightPassThreshold", parameters->warpBrightPassThreshold);
    
    ofSetColor(255, 255, 255, 255);
    video.draw(videoRectangle);
    
    warpApplyShader.end();
    
    displacementFbo.getTexture().unbind(1);
    video.getTexture().unbind(0);
    
    // === Swap displacement buffers for next frame ===
    std::swap(displacementFbo, previousDisplacementFbo);
}


void RenderApp::renderVideoStandard() {
    // Standard video rendering without warp (pass-through)

    video.getTexture().bind(0);

    videoShader.begin();

    videoShader.setUniform4f("videoColor",
        parameters->videoColor.get().r / 255.0f,
        parameters->videoColor.get().g / 255.0f,
        parameters->videoColor.get().b / 255.0f,
        parameters->videopreviewVisibility);

    videoShader.setUniformTexture("tex0", video.getTexture(), 0);
    videoShader.setUniform1f("time", ofGetElapsedTimef());
    videoShader.setUniform2f("resolution", videoRectangle.width, videoRectangle.height);

    ofSetColor(255, 255, 255, 255);
    video.draw(videoRectangle);

    videoShader.end();
    video.getTexture().unbind(0);
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

    // trails
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

    setupFBOs();
    setupWarpFBOs();

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));
}

//--------------------------------------------------------------
void RenderApp::setupParticleBuffers() {
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
}
