#include "simulator.h"

/// <summary>
/// assign params pointer and listeners to value changes
/// </summary>
/// <param name="params">pointer from the gui structure</param>

void Simulator::setup(Gui::SimulationParameters* params) {
    parameters = params;
    parameters->ammount.addListener(this, &Simulator::onGUIChangeAmmount);

    // to-do: fix bug. its taking window dimentions from the simulator parent app (mainApp)
    //        needs to get them from RenderApp... so maybe a listener? or move everything to a single window app and decouple GUI by using imgui 
    updateWorldSize(ofGetWidth(), ofGetHeight());
    generateParticles(parameters->ammount);
}


void Simulator::update() {
    for (auto& particle : particles) {
        particle.x += 0.01 * particle.a;
        particle.y += 0.01 * particle.b;

        if (particle.x > ofGetWidth() || particle.x < 0) {
            particle.a *= -1.0f;
            particle.x = ofClamp(particle.x, 0, ofGetWidth());
        }
        if (particle.y > ofGetHeight() || particle.y < 0) {
            particle.b *= -1.0f;
            particle.y = ofClamp(particle.y, 0, ofGetHeight());
        }
    }
}


void Simulator::generateParticles(int ammount) {
    ofLog() << "Simulator::generateParticles generating particles";
    particles.resize(ammount);
    for (int i = 0; i < ammount; i++) {
        if (particles[i].x == NULL) {
            glm::vec4 p;
            p.x = ofRandomWidth();
            p.y = ofRandomHeight();
            p.a = ofRandom(-25.0f, 25.0f) * parameters->momentum;
            p.b = ofRandom(-25.0f, 25.0f) * parameters->momentum;
            particles[i] = p;
        }
    }
}


#pragma region Listeners

void Simulator::onGUIChangeAmmount(int& value) {
    generateParticles(value);
}


void Simulator::updateWorldSize(int _width, int _height) {
    width = _width;
    height = _height;
}

void Simulator::recieveFrame(ofxCvGrayscaleImage frame) {
    return;
}


#pragma endregion


