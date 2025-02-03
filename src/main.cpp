#include "ofMain.h"
#include "ofApp.h"
#include "render/RenderApp.h"

#include "ofAppGLFWWindow.h"

//========================================================================
int main()
{
    ofGLFWWindowSettings settings;

    // render window
    settings.setSize(27*30, 27*30);
    settings.setPosition(glm::vec2(36*30,50));
    settings.resizable = true;
    settings.setGLVersion(3, 2);
    auto renderWindow = ofCreateWindow(settings);
    shared_ptr<RenderApp> renderApp(new RenderApp);



    // gui window
    settings.setSize(34*30, 33*30);
    settings.setPosition(glm::vec2(0,50));
    settings.resizable = true;
    auto mainWindow = ofCreateWindow(settings);
    shared_ptr<ofApp> mainApp(new ofApp);




    // PARAMETERS
    // this is how parameters are linked in between ofApps (main/gui and render)

    renderApp->globalParameters = &(mainApp->gui);
    renderApp->parameters = &(mainApp->gui.renderParameters);

    // listener to recieve window resizes from the render window (not used now)
    // ofAddListener(renderApp->viewportResizeEvent, mainApp.get(), &ofApp::onViewportResizeEvent);

    // this is the connection between the simulator and the render
    renderApp->particles = &(mainApp->simulator.particles.active);
    renderApp->simulator = &(mainApp->simulator);

    // ** note that parameters are not defined and exposed by its system (class) as the oF common way
    //    all params are defined on the GUI class in GUI ofApp, then linked to each system
	//    this is because parameter are shared between systems, so it make sense to have them in a single common place (the gui)
    //    i.e. the render may use the original size of the particles in the simulation

    //       TO-DO: may worth exploring how to link values in between parameters (i.e. move particle size affects render particle size * 10)
    //       TO-DO: may leave globalParameters as externals, but define render params in render class, and GuiApp should save references to each param-system


    ofRunApp(mainWindow, mainApp);
    ofRunApp(renderWindow, renderApp);

    ofRunMainLoop();
}

//extern "C" {
//    _declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
//}