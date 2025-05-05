#include "GuiApp.h"



void GuiApp::setup()
{
    ofBackground(0);
    fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight(), GL_RGBA32F_ARB);
    ofEnableSmoothing();

    gui.setupFlexBoxLayout();

    // a "pre"-allocation of the preview ofImages to allow control linking, but actual allocation is done by the Camera::setFrameSizes
    cameraParameters.previewSource.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewSegment.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewBackground.allocate(1, 1, OF_IMAGE_GRAYSCALE);

    allParameters = { &simulationParameters, &renderParameters, &cameraParameters }; // presets are handled at the presetsPanel
	presetManager.setup(allParameters);
	presetManager.setFolderPath("data\\presets\\");

    // the panels
    particlesPanel.setup(gui, simulationParameters);
    systemstatsPanel.setup(gui, simulationParameters);
    simulationPanel.setup(gui, simulationParameters);
    videoOriginPanel.setup(gui, cameraParameters);
    videoProcessingPanel.setup(gui, cameraParameters);
    renderPanel.setup(gui, renderParameters);
	sequencePanel.setup(gui, &presetsParameters, presetManager);
    presetsPanel.setup(gui, &presetsParameters, presetManager, simulationParameters, cameraParameters, renderParameters);


    //parameters->previewRender.grabScreen(0, 0, ofGetWidth(), ofGetHeight());

}

void GuiApp::update() 
{
    presetsPanel.update();

    float colorProgress = ofxeasing::map_clamp(bgChangeFrequency++, 0, bgChangeDuration, 0, 1, &ofxeasing::linear::easeInOut);
    if (bgChangeFrequency == bgChangeDuration) bgChangeFrequency = 0;

    if (colorProgress == 0) {
        //bgTargetColor1 = bgColors[ofRandom(bgColors.size())];
        //bgTargetColor2 = bgColors[ofRandom(bgColors.size())];
        bgTargetColor1 = renderParameters.color;
        bgTargetColor2 = renderParameters.videoColor;
        bgStartColor1 = bgColor1;
        bgStartColor2 = bgColor2;
    }
    //bgColor1 = bgStartColor1.getLerped(bgTargetColor1, colorProgress);
    //bgColor2 = bgStartColor2.getLerped(bgTargetColor2, colorProgress);

    bgColor1 = bgStartColor1.getLerped(renderParameters.color, colorProgress);
    bgColor2 = bgStartColor2.getLerped(renderParameters.videoColor, colorProgress);
}


void GuiApp::draw()
{
    ofBackgroundGradient(bgColor2, bgColor1, OF_GRADIENT_LINEAR);
    fbo.begin();

    ofSetColor(255, 150);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

    // draw lines
    EsenciaPanelBase::drawLineBetween(videoOriginPanel, videoProcessingPanel);
    EsenciaPanelBase::drawLineBetween(videoProcessingPanel, simulationPanel);
    EsenciaPanelBase::drawLineBetween(particlesPanel, simulationPanel);
    EsenciaPanelBase::drawLineBetween(simulationPanel, renderPanel);
    EsenciaPanelBase::drawLineBetween(presetsPanel, sequencePanel);

    fbo.end();
    fbo.draw(0,0);
}



void GuiApp::keyReleased(ofKeyEventArgs& e) {
    presetsPanel.keyReleased(e);
    sequencePanel.keyReleased(e);
    
    if (e.keycode == 'S' && e.hasModifier(OF_KEY_SHIFT)) {
        ofSaveFrame(true);
    }    
}


/// <summary>
/// on window resized event
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void GuiApp::windowResized(int _width, int _height) {
    fbo.allocate(_width, _height);
}






