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

    // the panels
    particlesPanel.setup(gui, simulationParameters);
    systemstatsPanel.setup(gui, simulationParameters);
    simulationPanel.setup(gui, simulationParameters);
    videoOriginPanel.setup(gui, cameraParameters);
    videoProcessingPanel.setup(gui, cameraParameters);
    renderPanel.setup(gui, renderParameters);
	presetsPanel.setup(gui, presetsParameters, simulationParameters, cameraParameters, renderParameters);

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
    fbo.end();
    fbo.draw(0,0);
}


// functions for the functionSlider (aka inverse expo slider)
static float linear(float x) {
	return x*10;
}

static float reverseLinear(float y) {
	return y/10;
}

static float exponentialFunction(float x) {
	return pow(10, x);
}

static float reversedExponentialFunction(float y) {
	return log10(y);
}

const int R = 100;
const float E = exp(1);
static float inverseExponentialFunction(float x) {
    float a = log( (R*E) +1);
    float b = PARTICLES_MAX / a;
    float c = log((E*x*R) +1);
    float d = b * c;
    return d;
}

static float reversedInverseExponentialFunction(float y) {
    float a = y / PARTICLES_MAX;
    float b = log((R*E) +1);
    float c = 1 / E;
    float d = a * b;
    float e = exp(d-1) - c;
    return e / R;
}




/// <summary>
/// Apply the values from a JSON file to the parameters
/// </summary>
/// <param name="jsonFilePath">Full path to the json file</param>
/// <param name="parameterGroups">A vector with references to the destination parameters</param>
static void applyJsonToParameters(const std::string& jsonFilePath, std::vector<ParametersBase*>& parameterGroups) {
	ofLog() << "Applying JSON to esencia parameters " << jsonFilePath;

    // Read the JSON file
    std::ifstream file(jsonFilePath);
    if (!file.is_open()) {
		ofLogError() << "Could not open JSON file";
    }

    // Parse the JSON file
    ofJson j;
    file >> j;

    // Iterate over all items in the JSON
    for (auto& [group, v] : j.items()) {  // first level is the parameter group

        // Iterate over all parameter groups to find the corresponding 1st level set from the json
        for (auto& paramGroup : parameterGroups) {
            if (paramGroup->parameterMap.find(group) != paramGroup->parameterMap.end()) {

				// iterate over all items in the group
                for (auto& [key, value] : j[group].items()) {

					// store actual param key and value
                    auto param = paramGroup->parameterMap[key];
                    if (param == nullptr) {
                        ofLog() << "Preset key " << key << " not found in " << group;
                        continue;
                    }

                    // cast to apply the value
                    const std::string paramTypeName = typeid(*param).name(); // get the type of the parameter to cast it
                    
                    if (paramTypeName == typeid(ofParameter<bool>).name()) {
                        dynamic_cast<ofParameter<bool>*>(param)->set(value.get<bool>());
                    }
                    else if (paramTypeName == typeid(ofParameter<int>).name()) {
                        dynamic_cast<ofParameter<int>*>(param)->set(value.get<int>());
                    }
                    else if (paramTypeName == typeid(ofParameter<float>).name()) {
                        dynamic_cast<ofParameter<float>*>(param)->set(value.get<float>());
                    }
                }
            }
        }
    }
}

static void applyPreset(int id, std::vector<ParametersBase*>& parameterGroups) {
    std::string jsonFilePath = "data\\presets\\" + std::to_string(id) + ".json";
    applyJsonToParameters(jsonFilePath, parameterGroups);
}



static void saveParametersToJson(const std::string& jsonFilePath, const std::vector<ParametersBase*>& parameterGroups) {
    ofJson j;

    for (const auto& paramGroup : parameterGroups) {
        ofJson groupJson;
        for (const auto& [key, param] : paramGroup->parameterMap) {
            if (param != nullptr) {
                const std::string paramTypeName = typeid(*param).name();

                if (paramTypeName == typeid(ofParameter<bool>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<bool>*>(param)->get();
                }
                else if (paramTypeName == typeid(ofParameter<int>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<int>*>(param)->get();
                }
                else if (paramTypeName == typeid(ofParameter<float>).name()) {
                    groupJson[key] = dynamic_cast<ofParameter<float>*>(param)->get();
                }
            }
        }
        j[paramGroup->parameterMap.begin()->first] = groupJson; // Assuming the first key is the group name
    }

    std::ofstream file(jsonFilePath);
    if (file.is_open()) {
        file << j.dump(4); // Pretty print with 4 spaces
        file.close();
    }
    else {
        ofLogError() << "Could not open JSON file for writing";
    }
}

static void savePreset(int id, const std::vector<ParametersBase*>& parameterGroups) {
    std::string idStr = (id < 10 ? "0" : "") + std::to_string(id);
    std::string jsonFilePath = "data\\presets\\" + idStr + ".json";
    saveParametersToJson(jsonFilePath, parameterGroups);
}





void GuiApp::drawLineBetween(EsenciaPanelBase &a, EsenciaPanelBase&b)
{
    const int BEZIER_DISTANCE_X = 40;
    const int BEZIER_RESOLUTION = 10;
    const int CIRCLE_RADIUS = 5;
    const int CIRCLE_RADIUS_2 = 3;
    const int TRIANGLE_SIZE = 8;

    int ox = a.panel->getPosition().x + a.panel->getWidth();
    int oy = a.panel->getPosition().y + a.panel->getHeight();
    int dx = b.panel->getPosition().x;
    int dy = b.panel->getPosition().y;

    ofPushMatrix();

    //ofSetLineWidth(10); // actually not working, not supported by opengl 3.2+

    ofSetColor(ofColor::paleGoldenRod, 200);
    ofFill();
    ofDrawCircle(ox + CIRCLE_RADIUS_2, oy - CIRCLE_RADIUS, CIRCLE_RADIUS);

    //ofDrawTriangle( ox + TRIANGLE_SIZE, oy,
    //                ox, oy - TRIANGLE_SIZE,
    //                ox, oy + TRIANGLE_SIZE);

    //ofNoFill();
    ofSetColor(ofColor::paleTurquoise, 200);
    ofDrawCircle(dx - CIRCLE_RADIUS_2, dy + CIRCLE_RADIUS, CIRCLE_RADIUS);

    //ofDrawTriangle( dx, dy,
    //                dx - TRIANGLE_SIZE, dy - TRIANGLE_SIZE,
    //                dx - TRIANGLE_SIZE, dy + TRIANGLE_SIZE);


    ofSetColor(ofColor::antiqueWhite);
    ofPolyline l;
    l.addVertex(ox, oy);
    l.bezierTo( ox + BEZIER_DISTANCE_X, oy,
        dx - BEZIER_DISTANCE_X, dy,
        dx, dy, BEZIER_RESOLUTION);
    l.draw();

    ofPopMatrix();
}



void GuiApp::keyReleased(ofKeyEventArgs& e) {
    //simulationParameters.ammount.enableEvents();
    //std::cout << "rad listeners" << simulationParameters.radius.getNumListeners() << std::endl;
    //std::cout << "amm listeners" << simulationParameters.ammount.getNumListeners() << std::endl;
    if (e.keycode == OF_KEY_DOWN) {
        simulationParameters.amount.set(ofRandom(150)); // demonstrate that changing the parameter value, it will update the gui accordingly.. except for the circular slider x_x
        simulationParameters.radius.set(ofRandom(30)); // demonstrate that changing the parameter value, it will update the gui accordingly.. except for the circular slider x_x
    }

    presetsPanel.keyReleased(e);

    if (e.keycode == 'A') {
        std::vector<ParametersBase*> allParameters = { &simulationParameters, &renderParameters, &cameraParameters };
		applyJsonToParameters("data\\presets\\01.json", allParameters);
    }

	presetsPanel.curPreset.set("1");
}


/// <summary>
/// on window resized event
/// </summary>
/// <param name="_width"></param>
/// <param name="_height"></param>
void GuiApp::windowResized(int _width, int _height) {
    fbo.allocate(_width, _height);
}






