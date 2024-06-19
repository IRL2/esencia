#include "Gui.h"

void Gui::setup()
// void Gui::setup(Camera *cam)
{
    inflateParticles();
    inflateSimulation();
    inflateRender();
    inflateVideo();

    inflateVideoProcessing();
    inflateVideoSources();
    inflateVideoBackground();
    inflateVideoClipping();
    inflateVideoPolygons();

    inflateSystemStats();

//     cameraGroup.add(ofParameter<string>().set("DEBUG"));
// #ifdef DEBUG_IMAGES
//     camera.add(cameraParameters.saveDebugImages.set("save debug images", false));
// #endif
// #ifdef RECORD_TESTING_VIDEO
//     camera.add(cameraParameters.recordTestingVideo.set("record testing video", false));
// #endif

    ofBackground(0);
    fbo.allocate(ofGetWindowWidth(), ofGetWindowHeight());
    ofEnableSmoothing();
}

void Gui::update() 
{
    // gaussian blur needs to be an odd value
    if (cameraParameters.gaussianBlur % 2 == 0) { cameraParameters.gaussianBlur = cameraParameters.gaussianBlur + 1; }
}

void Gui::draw()
{
    fbo.begin();
        ofBackgroundGradient(ofColor::darkSlateGray, ofColor::lightGoldenRodYellow, OF_GRADIENT_LINEAR);
        drawLineBetween(*videoPanel, *simulationPanel);
        drawLineBetween(*particlesPanel, *simulationPanel);
        drawLineBetween(*simulationPanel, *renderPanel);
    fbo.end();
    fbo.draw(0,0);
}


void Gui::inflateParticles()
{
    particlesPanel = gui.addPanel("particles");
    particlesPanel->add(simulationParameters.ammount.set("ammount of particles", 50, 1, 100));
    particlesPanel->add(simulationParameters.radius.set("particle radius", 10, 1, 50));

    particlesPanel->setPosition(30, 30);
    particlesPanel->setWidth(8*30);
    particlesPanel->setBackgroundColor(ofColor(200, 20, 20, 100));
    particlesPanel->setBorderColor(ofColor::red);
}

void Gui::inflateSimulation()
{
    /// SIMULATION
    simulationPanel = gui.addPanel("simulation");
    simulationPanel->add(simulationParameters.applyThermostat.set("apply thermostat", true));
    simulationPanel->add(simulationParameters.targetTemperature.set("equilibrium temperature", 25000.0, 1000.0, 1000000.0));
    simulationPanel->add(simulationParameters.coupling.set("coupling", 0.5, 0.1, 1.0));

    simulationPanel->setPosition(12*30, 7*30);
    simulationPanel->setWidth(11*30);
}

void Gui::inflateRender()
{
    renderPanel = gui.addPanel("render");
    renderPanel->add(renderParameters.color.set("particle color", ofColor(77, 130, 200)));
    renderPanel->add(renderParameters.useShaders.set("use shaders", false));
    renderPanel->add(renderParameters.useFaketrails.set("use fake trails", false));
    // TODO: add particle size factor

    renderPanel->setPosition(25*30, 8*30);
    renderPanel->setWidth(8*30);
    renderPanel->setBackgroundColor(ofColor(100, 20, 100, 100));
    renderPanel->setBorderColor(ofColor::purple);
}


void Gui::inflateVideo()
{
    videoPanel = gui.addPanel("video");
    videoPanel->maximize();
    videoPanel->setPosition(30, 8*30);
    videoPanel->setWidth(8*30);
    videoPanel->setBackgroundColor(ofColor(30, 30, 200, 100));
    videoPanel->setBorderColor(ofColor::blue);
    videoPanel->setAttribute("border-radius", 10);
}

void Gui::inflateVideoSources()
{
    ofxGuiGroup *p = videoPanel->addGroup("sources");
    p->add(cameraParameters._sourceOrbbec.set("orbbec camera", false));
    p->add(cameraParameters._sourceVideofile.set("video file", false));
    p->setWidth(8*30);
    p->minimize();

    // TODO: display collapsed image from the source
}

void Gui::inflateVideoClipping()
{
    // DEPTH CLIPPING
    ofxGuiGroup *p = videoPanel->addGroup("depth clipping");
    p->add(cameraParameters.clipFar.set("far clipping", 172, 0, 255));
    p->add(cameraParameters.clipNear.set("near clipping", 20, 0, 255));
    p->setWidth(8*30);
    p->minimize();

    // TODO: use a single range slider
}

void Gui::inflateVideoProcessing(){
    ofxGuiGroup *p = videoPanel->addGroup("processing");
    p->setWidth(8*30);
    
    // cameraProcessingPanel = gui.addPanel("video processing");
    p->add(cameraParameters.gaussianBlur.set("final gaussian blur", 0, 0, 100));
    p->add(cameraParameters.floodfillHoles.set("floodfill holes", false));
    // p->minimize();
}

void Gui::inflateVideoBackground()
{
    ofxGuiGroup *p = videoPanel->addGroup("background");

    // p = gui.addPanel("background");
    p->add(cameraParameters.startBackgroundReference.set("take background reference", false));
    p->add(cameraParameters.useMask.set("mask extraction", false));
    p->setWidth(8*30);
    p->minimize();
}

void Gui::inflateVideoPolygons(){
    ofxGuiGroup *p = videoPanel->addGroup("polygons");

    p->add(cameraParameters.blobMinArea.set("minArea Blobs", 0.05f, 0.0f, 0.3f));
    p->add(cameraParameters.blobMaxArea.set("maxArea Blobs", 0.8f, 0.5f, 1.0f));
    p->add(cameraParameters.nConsidered.set("maximum blobs number consider", 8, 0, 64));
    p->add(cameraParameters.showPolygons.set("show polygons", false));
    p->add(cameraParameters.fillHolesOnPolygons.set("find holes on polygon", true));
    p->add(cameraParameters.polygonTolerance.set("polygonApproximation", 1, 0, 5));
    p->setWidth(8*30);
    p->minimize();
}


void Gui::inflateSystemStats()
{
    ofxGuiGroup *p = gui.addGroup("performance");
    p->addFpsPlotter();

    p->setWidth(8*30);
    p->setPosition(ofGetWindowSize().x - (9*30), (1*30));
    p->setDraggable(true);

    systemstatsGroup = p;
}


void Gui::inflatePresets(){

}




void Gui::drawLineBetween(ofxGuiPanel &a, ofxGuiPanel &b)
{
    const int BEZIER_DISTANCE_X = 30;
    const int BEZIER_RESOLUTION = 10;
    const int CIRCLE_RADIUS = 4;
    const int CIRCLE_RADIUS_2 = CIRCLE_RADIUS/2;

    ofSetLineWidth(10); // actually not working, not supported by opengl 3.2+
    ofSetColor(ofColor::paleGoldenRod);

    int ox = a.getPosition().x + a.getWidth();
    int oy = a.getPosition().y + a.getHeight();
    int dx = b.getPosition().x;
    int dy = b.getPosition().y;

    ofCircle(ox - CIRCLE_RADIUS_2, oy - 2, CIRCLE_RADIUS);
    ofCircle(dx + CIRCLE_RADIUS_2, dy + 2, CIRCLE_RADIUS);

    ofPolyline l;
    l.addVertex(ox, oy);
    l.bezierTo( ox + BEZIER_DISTANCE_X, oy,
        dx - BEZIER_DISTANCE_X, dy,
        dx, dy, BEZIER_RESOLUTION);
    l.draw();
}