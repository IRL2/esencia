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

    #ifdef DEBUG_IMAGES
        cameraGroup.add(ofParameter<string>().set("DEBUG"));
        camera.add(cameraParameters.saveDebugImages.set("save debug images", false));
    #endif
    #ifdef RECORD_TESTING_VIDEO
        camera.add(cameraParameters.recordTestingVideo.set("record testing video", false));
    #endif

}

void GuiApp::update() 
{
    presetsPanel.update();
}


void GuiApp::draw()
{
    fbo.begin();
    //ofBackgroundGradient(ofColor::darkSlateGray, ofColor::lightGoldenRodYellow, OF_GRADIENT_LINEAR);
    ofBackgroundGradient(ofColor::darkSlateGray, ofColor::darkSalmon, OF_GRADIENT_LINEAR);

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
}


/// <summary>
/// on window resized event
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void GuiApp::windowResized(int _width, int _height) {
    fbo.allocate(_width, _height);
}






