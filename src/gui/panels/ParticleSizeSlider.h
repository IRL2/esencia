#include "ofxGuiExtended.h"

using namespace std;

class ParticleSizeSlider : public ofxGuiIntSlider {
private:
	const int WIDTH_OFFSET = 60;
	int width = { 0 };
	bool previewOffsetCalculated = { false };

public:
	ParticleSizeSlider(ofParameter<int>& _val, const ofJson& config = ofJson());

	//template<typename T>
	//void localValueChanged(T& val);
	void localValueChanged(int& val);

protected:
	virtual void render() override;
	virtual void onUpdate();
};

inline ParticleSizeSlider::ParticleSizeSlider(ofParameter<int>& _val, const ofJson& config)
{
	value.makeReferenceTo(_val);
	value.addListener(this, &ParticleSizeSlider::localValueChanged);
	setup();
	_setConfig(config);
}

//template<typename T>
//void localValueChanged(T& val){
//	value = val;
//	setNeedsRedraw();
//}
void ParticleSizeSlider::localValueChanged(int& val) {
	value = val;
	setNeedsRedraw();
}


void ParticleSizeSlider::render() {
	ofxGuiIntSlider::render();
	ofSetColor(this->getBorderColor(), 100);
	ofCircle(2 + this->getWidth() + WIDTH_OFFSET / 2, getHeight() / 2, value);
}


void ParticleSizeSlider::onUpdate() {
	if (width == 0) {
		width = getWidth() - WIDTH_OFFSET - 2;
		//std::cout << "calculating width:" << width << std::endl;
	}
	if (width != getWidth()) {
		this->setLayoutWidth(width, false);
		//std::cout << "setting new layout width:" << width << std::endl;
	}
}
