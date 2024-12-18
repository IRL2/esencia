#include "Gui.h"



void GuiApp::setup()
{
    ofBackground(0);
    fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
    ofEnableSmoothing();

    // a "pre"-allocation of the preview ofImages to allow control linking, but actual allocation is done by the Camera::setFrameSizes
    cameraParameters.previewSource.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewSegment.allocate(1, 1, OF_IMAGE_GRAYSCALE);
    cameraParameters.previewBackground.allocate(1, 1, OF_IMAGE_GRAYSCALE);

    configureParticlesPanel(1, 1, 8, 0);
    configureVideoinitialPanel(1, 9, 8, 0);
    configureVideoprocessingPanel(13, 8, 8, 0);
    configureSimulationPanel(25, 3, 7, 0);
    configureRenderPanel(25, 12, 8, 0);
    configureSystemstatsPanel(12, 1, 8, 0);

//     cameraGroup.add(ofParameter<string>().set("DEBUG"));
// #ifdef DEBUG_IMAGES
//     camera.add(cameraParameters.saveDebugImages.set("save debug images", false));
// #endif
// #ifdef RECORD_TESTING_VIDEO
//     camera.add(cameraParameters.recordTestingVideo.set("record testing video", false));
// #endif
}

void GuiApp::update() 
{
    // gaussian blur needs to be an odd value
    if (cameraParameters.gaussianBlur % 2 == 0) { cameraParameters.gaussianBlur = cameraParameters.gaussianBlur + 1; }
}


void GuiApp::draw()
{
    fbo.begin();
        ofBackgroundGradient(ofColor::darkSlateGray, ofColor::lightGoldenRodYellow, OF_GRADIENT_LINEAR);

        drawLineBetween(*videoOriginPanel, *videoProcessPanel);
        drawLineBetween(*videoProcessPanel, *simulationPanel);
        drawLineBetween(*particlesPanel, *simulationPanel);
        drawLineBetween(*simulationPanel, *renderPanel);
    fbo.end();
    fbo.draw(0,0);
}


// functions for the functionSlider (aka inverse expo slider)
float linear(float x) {
	return x*10;
}

float reverseLinear(float y) {
	return y/10;
}

float exponentialFunction(float x) {
	return pow(10, x);
}

float reversedExponentialFunction(float y) {
	return log10(y);
}

const int R = 100;
const float E = exp(1);
float inverseExponentialFunction(float x) {
    float a = log( (R*E) +1);
    float b = PARTICLES_MAX / a;
    float c = log((E*x*R) +1);
    float d = b * c;
    return d;
}

float reversedInverseExponentialFunction(float y) {
    float a = y / PARTICLES_MAX;
    float b = log((R*E) +1);
    float c = 1 / E;
    float d = a * b;
    float e = exp(d-1) - c;
    return e / R;
}



void GuiApp::configureParticlesPanel(int x, int y, int w, int h)
{
    particlesPanel = gui.addPanel("particles");
	
    // this one uses exponential sliders
    ofxGuiFloatFunctionSlider* functionAmmount = particlesPanel->add<ofxGuiFloatFunctionSlider>(simulationParameters.amount.set("amount", 120, PARTICLES_MIN, PARTICLES_MAX) , ofJson({{"type", "circular"}, {"width", 180}, {"height", 130}, {"precision", 0}}) );
    functionAmmount->setFunctions(inverseExponentialFunction, reversedInverseExponentialFunction);

    particlesPanel->add(simulationParameters.radius.set("scale", 3, 1, 30), ofJson({{"height", 50}, {"precision", 0}}));

    particlesPanel->setBackgroundColor(ofColor(200, 20, 20, 100));
    particlesPanel->loadTheme("support/gui-styles.json", true);

    particlesPanel->setPosition(x*30, y*30);
    particlesPanel->setWidth(w*30);
}


/// SIMULATION
void GuiApp::configureSimulationPanel(int x, int y, int w, int h)
{
    simulationPanel = gui.addPanel("simulation");

    simulationPanel->add(simulationParameters.applyThermostat.set("apply thermostat", true));

    ofxGuiContainer *p = simulationPanel->addContainer("" , ofJson({{"direction", "horizontal"}}) );
    p->add(simulationParameters.targetTemperature.set("\nequilibrium\ntemperature", 25000.0, 1000.0, 100000.0) , ofJson({{"width", 100}, {"height", 150}, {"precision", 0}}));
    p->add(simulationParameters.coupling.set("Berendsen\nthermostat\ncoupling", 0.5, 0.1, 1.0), ofJson({{"width", 100}, {"height", 150}, {"precision", 3}}));

    simulationPanel->setBackgroundColor(ofColor(180, 180, 180, 100));
    simulationPanel->loadTheme("support/gui-styles.json", true);

    simulationPanel->setPosition(x*30, y*30);
    simulationPanel->setWidth(w*30);
}


void GuiApp::configureRenderPanel(int x, int y, int w, int h)
{
    renderPanel = gui.addPanel("render");
    renderPanel->add(renderParameters.color.set("particle color", ofColor(77, 130, 200)))->minimize();
    renderPanel->add(renderParameters.useShaders.set("use shaders", false));

    // video
    ofxGuiGroup *vp = renderPanel->addGroup("video");
    vp->add(renderParameters.showVideoPreview.set("visible", false));
    vp->add(renderParameters.videopreviewVisibility.set("overlay", 0.3, 0.0, 1.0));
    vp->add(renderParameters.videoColor.set("color", ofColor::white))->minimize();

    // trails
    ofxGuiGroup *ofTrialsPanel = renderPanel->addGroup("trails");
    ofTrialsPanel->add(renderParameters.useFaketrails.set("enable", false));
    ofTrialsPanel->add(renderParameters.fakeTrialsVisibility.set("overlay", 0.05, 0.0, 0.3));

    // TODO: add particle size factor
    renderPanel->setBackgroundColor(ofColor(100, 20, 100, 100));
    renderPanel->loadTheme("support/gui-styles.json", true);

    renderPanel->setPosition(x*30, y*30);
    renderPanel->setWidth(w*30);
}


void GuiApp::configureVideoinitialPanel(int x, int y, int w, int h)
{
    videoOriginPanel = gui.addPanel("video origin");
    videoOriginPanel->setPosition(x*30, y*30);
    videoOriginPanel->setWidth(w*30);
    videoOriginPanel->loadTheme("support/gui-styles.json", true);
    videoOriginPanel->setBackgroundColor(ofColor(30, 30, 200, 100));

    // SOURCES
    ofxGuiGroup *cameraSourcePanel = videoOriginPanel->addGroup("sources");
    cameraSourcePanel->add<ofxGuiGraphics>("source", &cameraParameters.previewSource.getTexture() , ofJson({{"height", 200}}));
    cameraSourcePanel->add(cameraParameters._sourceOrbbec.set("orbbec camera", false));
    cameraSourcePanel->add(cameraParameters._sourceVideofile.set("video file", false));
    cameraSourcePanel->setWidth(w*30);
    cameraSourcePanel->minimize();
    
    // DEPTH CLIPPING
    ofxGuiGroup *cameraClippingPanel = videoOriginPanel->addGroup("depth clipping");
    cameraParameters.clipNear.set("visibility range", 20, 0, 255);
    cameraParameters.clipFar.set(170);
    cameraClippingPanel->add<ofxGuiIntRangeSlider>(cameraParameters.clipNear, cameraParameters.clipFar);
    cameraClippingPanel->setWidth(w*30);
    cameraClippingPanel->minimize();
    // TODO: use a single range slider

    // BACKGROUND
    ofxGuiGroup *cameraBackgroundPanel = videoOriginPanel->addGroup("background");
    cameraBackgroundPanel->add<ofxGuiGraphics>("reference", &cameraParameters.previewBackground.getTexture() , ofJson({{"height", 200}}));
    cameraBackgroundPanel->add<ofxGuiButton>(cameraParameters.startBackgroundReference.set("grab background frame", false), ofJson({{"type", "fullsize"}, {"text-align", "center"}}));

    cameraBackgroundPanel->setWidth(w*30);
    cameraBackgroundPanel->minimize();
}


void GuiApp::configureVideoprocessingPanel(int x, int y, int w, int h)
{
    videoProcessPanel = gui.addPanel("video processing");
    videoProcessPanel->setPosition(x*30, y*30);
    videoProcessPanel->setWidth(w*30);

    videoProcessPanel->loadTheme("support/gui-styles.json", true);
    videoProcessPanel->setBackgroundColor(ofColor(30, 30, 200, 100));
    videoProcessPanel->setAttribute("border-width", 10);


    ofxGuiGroup *cameraSourcePreview = videoProcessPanel->addGroup("preview");
    cameraSourcePreview->setWidth(w*30);
    cameraSourcePreview->add<ofxGuiGraphics>("segment", &cameraParameters.previewSegment.getTexture() , ofJson({{"height", 200}}));

    // PROCESSING
    ofxGuiGroup *cameraProcessingPanel = videoProcessPanel->addGroup("processing");
    cameraProcessingPanel->add(cameraParameters.gaussianBlur.set("final gaussian blur", 0, 0, 130));
    cameraProcessingPanel->add(cameraParameters.floodfillHoles.set("floodfill holes", false));
    cameraProcessingPanel->add(cameraParameters.useMask.set("preserve depth", false));
    cameraProcessingPanel->setWidth(w*30);

    // POLYGONS
    ofxGuiGroup *cameraPolygonsPanel = videoProcessPanel->addGroup("polygons");
    cameraPolygonsPanel->add(cameraParameters.blobMinArea.set("min size blob", 0.05f, 0.0f, 0.3f));
    cameraPolygonsPanel->add(cameraParameters.blobMaxArea.set("max size blob", 0.8f, 0.5f, 1.0f));
    cameraPolygonsPanel->add(cameraParameters.nConsidered.set("maximum blobs", 8, 0, 64));
    cameraPolygonsPanel->add(cameraParameters.showPolygons.set("show polygons", false));
    cameraPolygonsPanel->add(cameraParameters.fillHolesOnPolygons.set("find holes on polygon", true));
    cameraPolygonsPanel->add(cameraParameters.polygonTolerance.set("polygon approximation", 1, 0, 5));
    cameraPolygonsPanel->setWidth(w*30);
    cameraPolygonsPanel->minimize();
}


void GuiApp::configureSystemstatsPanel(int x, int y, int w, int h)
{
    ofxGuiPanel *statsPanel = gui.addPanel("performance");
    ofxGuiContainer *p = statsPanel->addContainer("", ofJson({{"direction", "horizontal"}}));
    p->addFpsPlotter(ofJson({{"width", 200}}));
    p->add(simulationParameters.limitedFps.set("30fps", true), ofJson({{"type", "radio"}}));

    simulationParameters.limitedFps.addListener(this, &GuiApp::limiteFps);

    statsPanel->loadTheme("support/gui-styles.json", true);

    statsPanel->setPosition(x*30, y*30);
    statsPanel->setWidth(w*30);
    // statsPanel->setDraggable(true);
}

void GuiApp::limiteFps(bool &v)
{
    if (v){
        ofSetFrameRate(30);
    } else {
        ofSetFrameRate(60);
    }
}

void GuiApp::configurePresetsPanel(int x, int y, int w, int h) {

}




void GuiApp::drawLineBetween(ofxGuiPanel &a, ofxGuiPanel &b)
{
    const int BEZIER_DISTANCE_X = 40;
    const int BEZIER_RESOLUTION = 10;
    const int CIRCLE_RADIUS = 4;
    const int CIRCLE_RADIUS_2 = 1;

    int ox = a.getPosition().x + a.getWidth();
    int oy = a.getPosition().y + a.getHeight();
    int dx = b.getPosition().x;
    int dy = b.getPosition().y;

    ofSetLineWidth(10); // actually not working, not supported by opengl 3.2+
    ofSetColor(ofColor::paleGoldenRod);
    ofFill();
    ofDrawCircle(ox - CIRCLE_RADIUS_2, oy - 2, CIRCLE_RADIUS);
    ofNoFill();
    ofCircle(ox - CIRCLE_RADIUS_2, oy - 2, CIRCLE_RADIUS);
    ofCircle(dx + CIRCLE_RADIUS_2, dy + 2, CIRCLE_RADIUS);

    ofSetColor(ofColor::antiqueWhite);
    ofPolyline l;
    l.addVertex(ox, oy);
    l.bezierTo( ox + BEZIER_DISTANCE_X, oy,
        dx - BEZIER_DISTANCE_X, dy,
        dx, dy, BEZIER_RESOLUTION);
    l.draw();
}


void GuiApp::keyReleased(int key)
{
    //simulationParameters.ammount.enableEvents();
    //std::cout << "rad listeners" << simulationParameters.radius.getNumListeners() << std::endl;
    //std::cout << "amm listeners" << simulationParameters.ammount.getNumListeners() << std::endl;
    if (key == OF_KEY_DOWN) {
        simulationParameters.amount.set(ofRandom(150)); // demonstrate that changing the parameter value, it will update the gui accordingly.. except for the circular slider x_x
        simulationParameters.radius.set(ofRandom(30)); // demonstrate that changing the parameter value, it will update the gui accordingly.. except for the circular slider x_x
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


