#include "Gui.h"

void Gui::setup()
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

    ofBackground(10);
}

void Gui::update() 
{
    // gaussian blur needs to be an odd value
    if (cameraParameters.gaussianBlur % 2 == 0) { cameraParameters.gaussianBlur = cameraParameters.gaussianBlur + 1; }
}

void Gui::draw()
{
    drawLineBetween(*videoPanel, *simulationPanel);
    drawLineBetween(*particlesPanel, *simulationPanel);
    drawLineBetween(*simulationPanel, *renderPanel);

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

    simulationPanel->setPosition(12*30, 11*30);
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
    videoPanel->setBackgroundColor(ofColor(30, 30, 200, 100));
    videoPanel->setBorderColor(ofColor::blue);
    videoPanel->setAttribute("border-radius", 10);
}

void Gui::inflateVideoSources()
{
    // VIDEO SOURCE
    ofxGuiGroup *p = videoPanel->addGroup("sources");
    p->add(cameraParameters._sourceOrbbec.set("orbbec camera", false));
    p->add(cameraParameters._sourceVideofile.set("video file", false));
    // p->setPosition(30,30);
    p->setWidth(8*30);
    p->minimize();

    // TODO: display collapsed image from the source

    // cameraSourcePanel = p;
}

void Gui::inflateVideoClipping()
{
    // DEPTH CLIPPING
    ofxGuiGroup *p = videoPanel->addGroup("depth clipping");
    p->add(cameraParameters.clipFar.set("far clipping", 172, 0, 255));
    p->add(cameraParameters.clipNear.set("near clipping", 20, 0, 255));
    // p->setPosition(30, 5*30);
    p->setWidth(8*30);
    p->minimize();

    // TODO: use a single range slider

    // cameraClippingPanel = p;

}

void Gui::inflateVideoProcessing(){
    ofxGuiGroup *p = videoPanel->addGroup("processing");
    
    // PROCESSING
    // cameraProcessingPanel = gui.addPanel("video processing");
    p->add(cameraParameters.gaussianBlur.set("final gaussian blur", 0, 0, 100));
    p->add(cameraParameters.floodfillHoles.set("floodfill holes", false));
    // p->minimize();

    // p->setPosition(60.0f, 160.0f);
    // p->setBackgroundColor(ofColor(30, 30, 200, 100));
    // p->setBorderColor(ofColor::blue);
}

void Gui::inflateVideoBackground()
{
    ofxGuiGroup *p = videoPanel->addGroup("background");

    // p = gui.addPanel("background");
    p->add(cameraParameters.startBackgroundReference.set("take background reference", false));
    p->add(cameraParameters.useMask.set("mask extraction", false));
    p->minimize();
    // cameraBackgroundPanel->add(cameraParameters.useMask.set("mask extraction", false));
    // cameraBackgroundPanel = p;
}

void Gui::inflateVideoPolygons(){
    ofxGuiGroup *p = videoPanel->addGroup("polygons");

    // cameraPolygonsPanel = gui.addPanel("polygons");
    p->add(cameraParameters.blobMinArea.set("minArea Blobs", 0.05f, 0.0f, 0.3f));
    p->add(cameraParameters.blobMaxArea.set("maxArea Blobs", 0.8f, 0.5f, 1.0f));
    p->add(cameraParameters.nConsidered.set("maximum blobs number consider", 8, 0, 64));
    p->add(cameraParameters.showPolygons.set("show polygons", false));
    p->add(cameraParameters.fillHolesOnPolygons.set("find holes on polygon", true));
    p->add(cameraParameters.polygonTolerance.set("polygonApproximation", 1, 0, 5));
    p->minimize();

    // videoPanel->add(p);
    // cameraPolygonsPanel->minimize();
}


void Gui::inflateSystemStats()
{
    ofxGuiGroup *p = gui.addGroup("performance");
    p->addFpsPlotter();

    p->setWidth(9*30);
    p->setPosition(ofGetWindowSize().x - (30*9), ofGetWindowSize().y - (30*3));
    // p->setPosition(ofGetWindowWidth()- (30*10), ofGetWindowHeight()- (30*2));

    systemstatsGroup = p;
}

void Gui::inflatePresets(){

}



void Gui::drawLineBetween(ofxGuiPanel &a, ofxGuiPanel &b)
{
    ofPolyline l;
    l.addVertex(a.getPosition().x + a.getWidth(), a.getPosition().y + a.getHeight());
    // l.addVertex(b.getPosition().x, b.getPosition().y);
    l.bezierTo( a.getPosition().x+ a.getWidth() +100, a.getPosition().y + a.getHeight(),
        b.getPosition().x-100, b.getPosition().y,
        b.getPosition().x, b.getPosition().y, 20);
    // l.curveTo(b.getPosition().x, b.getPosition().y, 0.0f);
    ofSetColor(ofColor::white);
    l.draw();
    // ofPath p;
    // p.setfil
}