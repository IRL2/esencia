#include "RenderApp.h"

ofImage video;
ofImage particleTexture;
ofColor BACKGROUND_COLOR = ofColor::black;
ofRectangle videoRectangle;

ofVbo particleVbo;
ofShader particleShader;
ofShader trailShader;
ofShader videoShader; 


ofFbo videoFbo;
ofFbo particlesFbo;
ofFbo compositeFbo;
ofFbo trailFbo;


std::vector<glm::vec3> particlePositions;
std::vector<float> particleSizes;

const bool INVEASTERNEGG = false;

//--------------------------------------------------------------
void RenderApp::setup()
{
    ofSetWindowTitle("esencia screen");

    ofDisableArbTex();
    ofEnableAlphaBlending();

    windowResized(ofGetWidth(), ofGetHeight());
    SetWindowPos(ofGetWin32Window(), HWND_DESKTOP, ofGetViewportWidth() - ofGetWidth(), 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
    ShowWindow(GetConsoleWindow(), SW_MINIMIZE);

    // Load the particle texture
    bool textureLoaded;
    if ((ofGetMonth() == 10 && ofGetDay() == 31) || INVEASTERNEGG) {
        textureLoaded = particleTexture.load("images/pumpkin.png");
    }
    else {
        textureLoaded = particleTexture.load("images/particle.png");
    }

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
    trailFbo.allocate(w, h, GL_RGBA32F_ARB);

    // Clear all FBOs
    clearFBO(videoFbo);
    clearFBO(particlesFbo);
    clearFBO(compositeFbo);
    clearFBO(trailFbo);


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
    ofBackground(0);

    // 1. Render video to dedicated FBO
    renderVideoPass();

    // 2. Render particles to dedicated FBO
    renderParticlesPass();

    // 3. Composite everything together
    renderCompositePass();

    // 4. Draw final result
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
void RenderApp::renderVideoWithShader() {
    // use disabled blending to preserve grayscale/blur values exactly as they are
    ofEnableBlendMode(OF_BLENDMODE_DISABLED);

    renderVideoStandard();
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

    glm::vec2 newSize = glm::vec2(_width, _height);
    parameters->windowSize.set(glm::vec2(_width, _height));
}

//--------------------------------------------------------------
void RenderApp::setupParticleBuffers() {
    particlePositions.reserve(10000);
    particleSizes.reserve(10000);
}
