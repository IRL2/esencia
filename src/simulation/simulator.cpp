#include "simulator.h"

/// <summary>
/// assign params pointer and listeners to value changes
/// </summary>
/// <param name="params">pointer from the gui structure</param>

void Simulator::setup(Gui::SimulationParameters* params, Gui* globalParams) {
    parameters = params;
    globalParameters = globalParams;

    parameters->ammount.addListener(this, &Simulator::onGUIChangeAmmount);

    globalParameters->renderParameters.windowSize.addListener(this, &Simulator::onRenderwindowResize);

    generateParticles(parameters->ammount);
}


void Simulator::update() {
    for (auto& particle : particles) {
        particle.x += 0.01 * particle.a;
        particle.y += 0.01 * particle.b;

        if (particle.x > width || particle.x < 0) {
            particle.a *= -1.0f;
            particle.x = ofClamp(particle.x, 0, width);
        }
        if (particle.y > height || particle.y < 0) {
            particle.b *= -1.0f;
            particle.y = ofClamp(particle.y, 0, height);
        }
    }
}


void Simulator::generateParticles(int ammount) {
    ofLog() << "Simulator::generateParticles generating particles";
    particles.resize(ammount);
    for (int i = 0; i < ammount; i++) {
        if (particles[i].x == NULL) {
            glm::vec4 p;
            p.x = ofRandom(0, width);
            p.y = ofRandom(0, height);
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

void Simulator::onRenderwindowResize(glm::vec2& worldSize) {
    updateWorldSize(worldSize.x, worldSize.y);
}

void Simulator::updateWorldSize(int _width, int _height) {
    ofLogNotice("Simulator::updateWorldSize()") << "wew size: " << _width << "," << _height;

    width = _width;
    height = _height;
    parameters->worldSize.set(glm::vec2(_width, _height)); // we may also use the parameters->worlSize.x directly
}

void Simulator::recieveFrame(ofxCvGrayscaleImage frame) {
    return;
}


#pragma endregion


