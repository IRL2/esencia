#pragma once

#include "ofxGuiExtended.h"
#include "EsenciaParameters.h"

const int PANELS_CIRCLE_RADIUS = 5;
const int PANELS_CIRCLE_OFFSET = PANELS_CIRCLE_RADIUS;
const int PANELS_CIRCLE_RESOLUTION = 8;
const int PANELS_BEZIER_PADDING = 40;
const int PANELS_BEZIER_RESOLUTION = 10;
const int PANELS_TRIANGLE_SIZE = 4;

class EsenciaPanelBase {


public:
	ofxGuiPanel* panel;

    void configVisuals(ofRectangle rect, ofColor color) {
        panel->loadTheme("support/gui-styles.json", true);
        panel->setBackgroundColor(color);

        panel->setPosition(rect.x * 30, rect.y * 30);
        panel->setWidth(rect.width * 30);
        //panel->setHeight(rect.height * 30);
    }

    static void drawLineBetween(EsenciaPanelBase& a, EsenciaPanelBase& b)
    {
        int ox = a.panel->getPosition().x + a.panel->getWidth() + PANELS_CIRCLE_RADIUS;
        int oy = a.panel->getPosition().y + a.panel->getHeight() - PANELS_CIRCLE_OFFSET;
        int dx = b.panel->getPosition().x - PANELS_CIRCLE_RADIUS;
        int dy = b.panel->getPosition().y + PANELS_CIRCLE_OFFSET;

		int originCircleX = ox - PANELS_CIRCLE_RADIUS;
		int originCircleY = oy - PANELS_CIRCLE_RADIUS;
		ofVec3f destinationArrowStart = ofVec3f(dx - PANELS_CIRCLE_RADIUS, dy + PANELS_TRIANGLE_SIZE);
		ofVec3f destinationArrowEnd   = ofVec3f(dx, dy + PANELS_TRIANGLE_SIZE);
        int bezierCX1 = ox + PANELS_BEZIER_PADDING;
        int bezierCY1 = oy;
		int bezierCX2 = dx - PANELS_BEZIER_PADDING - PANELS_TRIANGLE_SIZE;
		int bezierCY2 = dy;
		int bezierX = dx - PANELS_TRIANGLE_SIZE;
		int bezierY = dy + PANELS_CIRCLE_RADIUS;

		// if origin is placed befor the destination then we need to adjust the bezier curve
		if (ox > dx) {
            bezierCY1 = oy + PANELS_BEZIER_PADDING;
			bezierCY2 = dy - PANELS_BEZIER_PADDING;
			if (oy > dy) {
				bezierCX1 = ox + PANELS_BEZIER_PADDING + PANELS_BEZIER_PADDING;
                bezierCY1 = oy;
				bezierCX2 = dx - PANELS_BEZIER_PADDING - PANELS_TRIANGLE_SIZE - PANELS_BEZIER_PADDING;
                bezierCY2 = dy;
			}
		}

        ofPushMatrix();

        // origin glyph
        ofSetColor(ofColor::paleGoldenRod, 200);
		ofSetCircleResolution(PANELS_CIRCLE_RESOLUTION);
        ofFill();
        ofDrawCircle(originCircleX, originCircleY, PANELS_CIRCLE_RADIUS);

        // destination glyph
        ofSetColor(ofColor::paleTurquoise, 200);
		ofDrawArrow(destinationArrowStart, destinationArrowEnd, PANELS_TRIANGLE_SIZE);

        ofSetColor(ofColor::khaki, 200);
        ofPolyline l;
        l.addVertex(originCircleX, originCircleY);
        l.bezierTo(bezierCX1, bezierCY1,
                   bezierCX2, bezierCY2,
                   bezierX, bezierY,
                   PANELS_BEZIER_RESOLUTION);
        l.draw();

        ofPopMatrix();
    }

};
