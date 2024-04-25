#include "Gui.h"

void Gui::setup()
{
    // todo: switch to ofxImGui for logarithmic slider support, and to use the graph widgets for cpu, fft, ...

    simulation.setName("SIMULATION");
    simulation.add(simulationParameters.ammount.set("ammount of particles", 50, 1, 100));
    simulation.add(simulationParameters.momentum.set("initial force factor", 4, 1, 10));
    simulation.add(simulationParameters.radius.set("particle radius", 10, 1, 50));

    render.setName("RENDER");
    render.add(renderParameters.size.set("particle size", 5, 1, 50));
    render.add(renderParameters.color.set("particle color", ofColor(77, 130, 200)));
    render.add(renderParameters.useShaders.set("use shaders", true));
    render.add(renderParameters.useFaketrails.set("use fake trails", false));

    
    camera.setName("CAMERA");
    
    camera.add(ofParameter<string>().set("BLOBS"));
    camera.add(cameraParameters.blobMinArea.set("minArea Blobs", 0.05f, 0.0f, 0.3f));
    camera.add(cameraParameters.blobMaxArea.set("maxArea Blobs", 0.8f, 0.5f, 1.0f));
    camera.add(cameraParameters.nConsidered.set("maximum blobs number consider", 8, 0, 64));

    camera.add(ofParameter<string>().set("POLYGONS"));
    camera.add(cameraParameters.showPolygons.set("show polygons", false));
    camera.add(cameraParameters.fillHolesOnPolygons.set("find holes on polygon", true));
    camera.add(cameraParameters.polygonTolerance.set("polygonApproximation", 1, 0, 5));

    camera.add(ofParameter<string>().set("THRESHOLDS"));
    camera.add(cameraParameters.enableClipping.set("Enable frame & bg clipping", true));
    camera.add(cameraParameters.clipFar.set("far clipping", 172, 0, 255));
    camera.add(cameraParameters.clipNear.set("near clipping", 20, 0, 255));

    camera.add(ofParameter<string>().set("BACKGROUND"));
    //camera.add(cameraParameters.backgroundSamples.set("background samples", 1, 1, 20));
    camera.add(cameraParameters.startBackgroundReference.set("take background reference", false));

    camera.add(ofParameter<string>().set("FINISHING"));
    camera.add(cameraParameters.floodfillHoles.set("floodfill holes", false));
    camera.add(cameraParameters.useMask.set("mask extraction", false));
    camera.add(cameraParameters.gaussianBlur.set("final gaussian blur", 41, 0, 100));

    camera.add(ofParameter<string>().set("DEBUG"));
#ifdef DEBUG_IMAGES
    camera.add(cameraParameters.saveDebugImages.set("save debug images", false));
#endif


    parameters.setName("  d( -_-)  s( ~_~) ");
    parameters.add(guiSeparator);
    parameters.add(simulation);
    parameters.add(guiSeparator);
    parameters.add(render);
    parameters.add(guiSeparator);
    parameters.add(camera);

    guiPanel.setup(parameters);
    guiPanel.setSize(ofGetWidth()-340, guiPanel.getHeight());
    guiPanel.setBackgroundColor(ofColor(100,100,100));
    guiPanel.setDefaultBackgroundColor(ofColor(100, 100, 100));
    guiPanel.setFillColor(ofColor(ofColor(100, 0, 0)));

    guiPanel.setPosition(330.0f, 40.0f);


    // each panel can be their own floating widget if
    // simulationPanel.setup(simulation);
    // renderPanel.setup(render);

    // GUI THEMING

    ofxGuiGroup* _simulgroup = &guiPanel.getGroup("SIMULATION");
    _simulgroup->setHeaderBackgroundColor(ofColor(200, 20, 20));
    _simulgroup->setBackgroundColor(ofColor(200, 20, 20));
    _simulgroup->setBorderColor(ofColor::red);

    ofxGuiGroup* _rendergroup = &guiPanel.getGroup("RENDER");
    _rendergroup->setHeaderBackgroundColor(ofColor(100, 20, 100));
    _rendergroup->setBackgroundColor(ofColor(100, 20, 100));
    _rendergroup->setBorderColor(ofColor::purple);

    ofxGuiGroup* _cameragroup = &guiPanel.getGroup("CAMERA");
    _cameragroup->setHeaderBackgroundColor(ofColor(30, 30, 200));
    _cameragroup->setBackgroundColor(ofColor(30, 30, 200));
    _cameragroup->setBorderColor(ofColor::blue);

    ofBackground(10);
}

void Gui::update() 
{
    // gaussian blur needs to be an odd value
    if (cameraParameters.gaussianBlur % 2 == 0) { cameraParameters.gaussianBlur = cameraParameters.gaussianBlur + 1; }
}

void Gui::draw()
{
    guiPanel.draw();
}