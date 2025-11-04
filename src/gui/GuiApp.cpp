#include "GuiApp.h"



void GuiApp::setup()
{
    ofBackground(0);
    fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
    ofEnableSmoothing();

    gui.setupFlexBoxLayout();

    // a "pre"-allocation of the preview ofImages to allow control linking, but actual allocation is done by the Camera::setFrameSizes
    cameraParameters.previewSource.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewSegment.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewBackground.allocate(1, 1, OF_IMAGE_GRAYSCALE);

    allParameters = { &simulationParameters, &renderParameters, &cameraParameters, &sonificationParameters }; // presets are handled at the presetsPanel
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
    presetsPanel.setup(gui, &presetsParameters, presetManager);
    audioPanel.setup(gui, &sonificationParameters, &simulationParameters);

    //parameters->previewRender.grabScreen(0, 0, ofGetWidth(), ofGetHeight());
    bgTargetColor1 = ofColor::white;
    bgTargetColor2 = ofColor::white;
    bgStartColor1 = ofColor::white;
    bgStartColor2 = ofColor::white;
}

void GuiApp::update() 
{
    presetsPanel.update();

	float colorProgress = ofxSEeasing::map_clamp(bgChangeFrequency++, 0, bgChangeDuration, 0, 1, &ofxSEeasing::easeInOutQuad);
    if (bgChangeFrequency == bgChangeDuration) bgChangeFrequency = 0;

    if (colorProgress == 0) {
        bgTargetColor1.setHsb(renderParameters.color.get().getHueAngle(), 200, 100);
        bgTargetColor2.setHsb(renderParameters.videoColor.get().getHueAngle(), 200, 100);
        bgStartColor1 = bgColor1;
        bgStartColor2 = bgColor2;
    }
    bgColor1 = bgStartColor1.getLerped(bgTargetColor1, colorProgress);
    bgColor2 = bgStartColor2.getLerped(bgTargetColor2, colorProgress);
}


void GuiApp::draw()
{
    fbo.begin();
    ofBackgroundGradient(bgColor2, bgColor1, OF_GRADIENT_LINEAR);

    // draw lines
    EsenciaPanelBase::drawLineBetween(videoOriginPanel, videoProcessingPanel);
    EsenciaPanelBase::drawLineBetween(particlesPanel, simulationPanel);
    EsenciaPanelBase::drawLineBetween(videoProcessingPanel, simulationPanel, 1, 0);
    EsenciaPanelBase::drawLineBetween(simulationPanel, simulationDataPanel, 0, 3);
    EsenciaPanelBase::drawLineBetween(simulationPanel, vacPanel, 0, 2);
    EsenciaPanelBase::drawLineBetween(simulationPanel, audioPanel, 0, 1);
    EsenciaPanelBase::drawLineBetween(simulationPanel, renderPanel, 0, 0);
    EsenciaPanelBase::drawLineBetween(presetsPanel, sequencePanel);

    fbo.end();
    fbo.draw(0,0);

    // panels draw themselves automatically after this draw event; they had a drawingg event listener added during setup
}




void GuiApp::keyReleased(ofKeyEventArgs& e) {
    presetsPanel.keyReleased(e);
    sequencePanel.keyReleased(e);
}


/// <summary>
/// on window resized event
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void GuiApp::windowResized(int _width, int _height) {
    fbo.allocate(_width, _height);
}

void GuiApp::setupVACPanel(Simulator* simulator) {
    if (simulator) {
        vacPanel.setup(gui, simulationParameters, sonificationParameters, simulator);
        ofLogNotice("GuiApp") << "VAC Panel setup completed";
    } else {
        ofLogError("GuiApp") << "simulator is null";
    }
}

void GuiApp::setupSimulationDataPanel(Simulator* simulator) {
    if (simulator) {
        simulationDataPanel.setup(gui, simulationParameters, simulator);
        ofLogNotice("GuiApp") << "Simulation Data Panel setup completed";
    } else {
        ofLogError("GuiApp") << "simulator is null";
    }
}
