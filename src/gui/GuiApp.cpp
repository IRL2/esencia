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

    // the panels
    particlesPanel.setup(gui, simulationParameters);
    systemstatsPanel.setup(gui, simulationParameters);
    simulationPanel.setup(gui, simulationParameters);
    videoOriginPanel.setup(gui, cameraParameters);
    videoProcessingPanel.setup(gui, cameraParameters);
    renderPanel.setup(gui, renderParameters);
	sequencePanel.setup(gui, &presetsParameters, presetManager);
    presetsPanel.setup(gui, &presetsParameters, simulationParameters, cameraParameters, renderParameters, presetManager);

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
    // gaussian blur needs to be an odd value
    // TODO: use a parameter addListener to monitor this rule
    if (cameraParameters.gaussianBlur % 2 == 0) { cameraParameters.gaussianBlur = cameraParameters.gaussianBlur + 1; }

    presetsPanel.update();
}


void GuiApp::draw()
{
    fbo.begin();
        ofBackgroundGradient(ofColor::darkSlateGray, ofColor::lightGoldenRodYellow, OF_GRADIENT_LINEAR);

        // draw lines
        drawLineBetween(videoOriginPanel, videoProcessingPanel);
        drawLineBetween(videoProcessingPanel, simulationPanel);
        drawLineBetween(particlesPanel, simulationPanel);
        drawLineBetween(simulationPanel, renderPanel);
		drawLineBetween(presetsPanel, sequencePanel);
    fbo.end();
    fbo.draw(0,0);
}




// TODO: this code should be moved to the EsenciaPanelBase class
//       should be called as EsenciaPanelBase::drawLineBetween(particlesPanel, simulationPanel);
// TODO: this code should be cleaned up
void GuiApp::drawLineBetween(EsenciaPanelBase &a, EsenciaPanelBase&b)
{
    const int BEZIER_DISTANCE_X = 40;
    const int BEZIER_RESOLUTION = 10;
    const int CIRCLE_RADIUS = 5;
    const int CIRCLE_RADIUS_2 = 5;
    const int TRIANGLE_SIZE = 4;
	const int CIRCLE_OFFSET_Y = 5;

    int ox = a.panel->getPosition().x + a.panel->getWidth() + CIRCLE_RADIUS_2;
    int oy = a.panel->getPosition().y + a.panel->getHeight() - CIRCLE_OFFSET_Y;
    int dx = b.panel->getPosition().x - CIRCLE_RADIUS_2;
    int dy = b.panel->getPosition().y + CIRCLE_OFFSET_Y;

    ofPushMatrix();

    //ofSetLineWidth(10); // actually not working, not supported by opengl 3.2+

    ofSetColor(ofColor::paleGoldenRod, 200);
    ofFill();
    ofDrawCircle(ox - CIRCLE_RADIUS, oy - CIRCLE_RADIUS_2, CIRCLE_RADIUS);

    //ofDrawTriangle( ox + TRIANGLE_SIZE, oy,
    //                ox, oy - TRIANGLE_SIZE,
    //                ox, oy + TRIANGLE_SIZE);

    //ofNoFill();
    ofSetColor(ofColor::paleTurquoise, 200);
    //ofDrawCircle(dx + CIRCLE_RADIUS, dy + CIRCLE_RADIUS_2, CIRCLE_RADIUS);

    //ofDrawTriangle( dx, dy,
    //                dx - TRIANGLE_SIZE, dy - TRIANGLE_SIZE,
    //                dx - TRIANGLE_SIZE, dy + TRIANGLE_SIZE);

	ofDrawArrow(ofVec3f(dx - CIRCLE_RADIUS_2, dy + CIRCLE_RADIUS_2), 
        ofVec3f(dx, dy + CIRCLE_RADIUS_2), 
        TRIANGLE_SIZE);

    ofSetColor(ofColor::antiqueWhite);
    ofPolyline l;
    l.addVertex(ox, oy - CIRCLE_RADIUS_2);
    l.bezierTo( ox + BEZIER_DISTANCE_X, oy,
        dx - BEZIER_DISTANCE_X - TRIANGLE_SIZE, dy,
        dx - TRIANGLE_SIZE, dy + CIRCLE_RADIUS_2,
        BEZIER_RESOLUTION);
    l.draw();

    ofPopMatrix();
}



void GuiApp::keyReleased(ofKeyEventArgs& e) {
    presetsPanel.keyReleased(e);
}


/// <summary>
/// on window resized event
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void GuiApp::windowResized(int _width, int _height) {
    fbo.allocate(_width, _height);
}






